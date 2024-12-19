#include "canvas.hpp"
#include <glad/glad.h>
#include <iostream>
#include "../utils/shaders.hpp"

Canvas::Canvas(int w, int h)
    : textureWidth(w), textureHeight(h),
      windowWidth(0), windowHeight(0),
      backgroundColor(Color(1.0f, 1.0f, 1.0f, 1.0f)),
      projection(1.0f), canvasScale(0.8f),
      VAO(0), VBO(0), EBO(0),
      activeLayerIndex(0) {
    // Create initial background layer
    addLayer("Background");
}

Canvas::~Canvas() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

bool Canvas::init() {
    try {
        // Setup shaders
        shader = std::make_unique<Shader>(Shaders::canvasVertexShader, 
                                        Shaders::canvasFragmentShader);
        
        // Setup quad for rendering
        setupQuad();
        
        // Initialize all layers
        for (auto& layer : layers) {
            if (!layer->init()) {
                return false;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error in Canvas::init(): " << e.what() << std::endl;
        return false;
    }
}

void Canvas::setupQuad() {
    float vertices[] = {
        // positions        // texture coords
        -1.0f,  1.0f,      0.0f, 1.0f,  // top left
         1.0f,  1.0f,      1.0f, 1.0f,  // top right
         1.0f, -1.0f,      1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f,      0.0f, 0.0f   // bottom left
    };
    
    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 
                         (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void Canvas::render() {
    saveViewport();
    glViewport(0, 0, windowWidth, windowHeight);
    
    // Clear the canvas area
    glEnable(GL_SCISSOR_TEST);
    int canvasX, canvasY, canvasWidth, canvasHeight;
    getCanvasRect(canvasX, canvasY, canvasWidth, canvasHeight);
    glScissor(canvasX, canvasY, canvasWidth, canvasHeight);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
    
    shader->use();
    glUniformMatrix4fv(glGetUniformLocation(shader->getProgram(), "projection"), 
                       1, GL_FALSE, &projection[0][0]);
    
    // Setup blending for layer compositing
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    // Render each visible layer from bottom to top
    for (const auto& layer : layers) {
        if (layer->getVisibility()) {
            glUniform1f(glGetUniformLocation(shader->getProgram(), "layerOpacity"), 
                       layer->getOpacity());

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, layer->getTexture());
            
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }

    // Render current stroke texture if available
    if (isStroking && currentStrokeTexture) {
        glUniform1f(glGetUniformLocation(shader->getProgram(), "layerOpacity"), 1.0f);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentStrokeTexture);
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    
    restoreViewport();
}

void Canvas::addLayer(const std::string& name) {
    auto layer = std::make_unique<Layer>(textureWidth, textureHeight, name);
    if (layer->init()) {
        layers.push_back(std::move(layer));
        activeLayerIndex = layers.size() - 1;
    }
}

void Canvas::removeLayer(size_t index) {
    if (index < layers.size() && layers.size() > 1) {  // Keep at least one layer
        layers.erase(layers.begin() + index);
        if (activeLayerIndex >= layers.size()) {
            activeLayerIndex = layers.size() - 1;
        }
    }
}

void Canvas::setActiveLayer(size_t index) {
    if (index < layers.size()) {
        activeLayerIndex = index;
    }
}

Layer* Canvas::getLayer(size_t index) {
    if (index < layers.size()) {
        return layers[index].get();
    }
    return nullptr;
}

void Canvas::clear() {
    for (auto& layer : layers) {
        layer->clear();
    }
}

void Canvas::resize(int wWidth, int wHeight) {
    windowWidth = wWidth;
    windowHeight = wHeight;
    updateProjection();
}

void Canvas::updateProjection() {
    // Compute display size based on texture aspect ratio
    float aspectRatio = (float)textureWidth / textureHeight;
    float windowAspect = (float)windowWidth / windowHeight;
    
    if (windowAspect > aspectRatio) {
        displayHeight = windowHeight * canvasScale;
        displayWidth = displayHeight * aspectRatio;
    } else {
        displayWidth = windowWidth * canvasScale;
        displayHeight = displayWidth / aspectRatio;
    }

    float scaleX = (float)displayWidth / windowWidth;
    float scaleY = (float)displayHeight / windowHeight;
    
    projection = glm::mat4(1.0f);
    projection = glm::scale(projection, glm::vec3(scaleX, scaleY, 1.0f));
}

void Canvas::saveViewport() {
    glGetIntegerv(GL_VIEWPORT, savedViewport);
}

void Canvas::restoreViewport() {
    glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);
}

void Canvas::getCanvasRect(int& x, int& y, int& w, int& h) const {
    float aspect = (float)textureWidth / textureHeight;
    float windowAspect = (float)windowWidth / windowHeight;
    
    if (aspect > windowAspect) {
        w = windowWidth * canvasScale;
        h = w / aspect;
    } else {
        h = windowHeight * canvasScale;
        w = h * aspect;
    }
    
    x = (windowWidth - w) / 2;
    y = (windowHeight - h) / 2;
}

bool Canvas::windowToCanvas(double windowX, double windowY, 
                          float& canvasX, float& canvasY) const {
    float canvasLeft = (windowWidth - displayWidth) * 0.5f;
    float canvasTop = (windowHeight - displayHeight) * 0.5f;
    
    if (windowX >= canvasLeft && windowX < canvasLeft + displayWidth &&
        windowY >= canvasTop && windowY < canvasTop + displayHeight) {
        canvasX = (windowX - canvasLeft) / displayWidth * textureWidth;
        canvasY = (windowY - canvasTop) / displayHeight * textureHeight;
        return true;
    }
    return false;
}