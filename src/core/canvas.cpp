#include "canvas.hpp"
#include <glad/glad.h>
#include <iostream>
#include "../utils/shaders.hpp"
#include "../commands/layer_commands.hpp"

Canvas::Canvas(int w, int h)
    : textureWidth(w), textureHeight(h),
      windowWidth(0), windowHeight(0),
      backgroundColor(Color(1.0f, 1.0f, 1.0f, 1.0f)),
      projection(1.0f), canvasScale(0.8f),
      VAO(0), VBO(0), EBO(0),
      activeLayerIndex(0),
      currentStrokeTexture(0),
      isStroking(false) {
    // Create initial background layer
    addLayer("Background");
    
    // Initialize command manager
    commandManager = std::make_unique<CommandManager>(this);
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
        
        // Setup composite buffer
        setupCompositeBuffer();

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

void Canvas::setupCompositeBuffer() {
    // Create framebuffer
    glGenFramebuffers(1, &compositeFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer);
    
    // Create texture
    glGenTextures(1, &compositeTexture);
    glBindTexture(GL_TEXTURE_2D, compositeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureWidth, textureHeight, 
                 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                          GL_TEXTURE_2D, compositeTexture, 0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Composite framebuffer is not complete!" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Canvas::render() {
    if (compositeFramebuffer == 0) {
        setupCompositeBuffer();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer);
    glViewport(0, 0, textureWidth, textureHeight);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    shader->use();
    glUniformMatrix4fv(glGetUniformLocation(shader->getProgram(), "projection"), 
                       1, GL_FALSE, &projection[0][0]);
    
    // Setup blending for layer compositing
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    // Render each visible layer from bottom to top
    glBindVertexArray(VAO);
    for (size_t i = 0; i < layers.size(); ++i) {
        auto layer = layers[i].get();
        if (layer->getVisibility()) {
            glUniform1f(glGetUniformLocation(shader->getProgram(), "layerOpacity"), 
                       layer->getOpacity());

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, layer->getTexture());
            
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            if (i == activeLayerIndex && isStroking && currentStrokeTexture) {
                glUniform1f(glGetUniformLocation(shader->getProgram(), "layerOpacity"), 
                            layer->getOpacity());
                
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, currentStrokeTexture);
                
                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Canvas::getCompositeTexture() const {
    return compositeTexture;
}

void Canvas::addLayer(const std::string& name, bool addToCommandStack) {
    auto layer = std::make_unique<Layer>(textureWidth, textureHeight, name, commandManager.get());
    if (layer->init()) {
        layers.push_back(std::move(layer));
        activeLayerIndex = layers.size() - 1;

        // Create command for adding the layer
        if (commandManager && addToCommandStack) {
            auto command = std::make_unique<AddLayerCommand>(name, layers.back()->getId());
            commandManager->addCommand(std::move(command));
        }
    }
}

void Canvas::removeLayer(size_t index, bool addToCommandStack) {
    if (index < layers.size() && layers.size() > 1) {  // Keep at least one layer
        // Create command for removing the layer
        if (commandManager && addToCommandStack) {
            auto command = std::make_unique<RemoveLayerCommand>(layers[index]->getId(), index);
            commandManager->addCommand(std::move(command));
        }
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

void Canvas::moveLayer(size_t sourceIdx, size_t targetIdx, bool addToCommandStack) {
    if (sourceIdx == targetIdx || 
        sourceIdx >= layers.size() || 
        targetIdx >= layers.size()) {
        return;
    }

    // Create command for moving the layer
    if (commandManager && addToCommandStack) {
        auto command = std::make_unique<MoveLayerCommand>(sourceIdx, targetIdx);
        commandManager->addCommand(std::move(command));
    }
    
    // Store the active layer index
    size_t activeIdx = activeLayerIndex;
    
    // Move the layer
    auto temp = std::move(layers[sourceIdx]);
    layers.erase(layers.begin() + sourceIdx);
    layers.insert(layers.begin() + targetIdx, std::move(temp));
    
    // Update active layer index if needed
    if (activeIdx == sourceIdx) {
        activeLayerIndex = targetIdx;
    } else if (activeIdx > sourceIdx && activeIdx <= targetIdx) {
        activeLayerIndex = activeIdx - 1;
    } else if (activeIdx < sourceIdx && activeIdx >= targetIdx) {
        activeLayerIndex = activeIdx + 1;
    }
}

void Canvas::duplicateLayer(size_t index, bool addToCommandStack) {
    if (index >= layers.size()) {
        return;
    }

    Layer* sourceLayer = layers[index].get();
    if (!sourceLayer) {
        return;
    }

    // Create a new layer with a copy of the name
    std::string newName = sourceLayer->getName() + " Copy";
    addLayer(newName);
    auto newLayer = std::make_unique<Layer>(textureWidth, textureHeight, newName);
    
    // Initialize the new layer
    if (!newLayer->init()) {
        return;
    }

    // Copy the layer properties
    newLayer->setOpacity(sourceLayer->getOpacity());
    newLayer->setBlendMode(sourceLayer->getBlendMode());
    newLayer->setVisibility(sourceLayer->getVisibility());

    // Copy the layer content
    PixelRect fullRect{0, 0, textureWidth, textureHeight};
    std::vector<float> pixels = sourceLayer->getPixels(fullRect);
    newLayer->setPixels(fullRect, pixels);

    // Create command for duplicating the layer
    if (commandManager && addToCommandStack) {
        auto command = std::make_unique<DuplicateLayerCommand>(sourceLayer->getId(), newLayer->getId());
        commandManager->addCommand(std::move(command));
    }

    // Insert the new layer after the source layer
    auto insertPos = layers.begin() + index + 1;
    layers.insert(insertPos, std::move(newLayer));

    // Update active layer index if needed
    if (activeLayerIndex > index) {
        activeLayerIndex++;
    }
}

Layer* Canvas::getLayer(size_t index) {
    if (index < layers.size()) {
        return layers[index].get();
    }
    return nullptr;
}

Layer* Canvas::getLayerById(const std::string& id) {
    for (auto& layer : layers) {
        if (layer->getId() == id) {
            return layer.get();
        }
    }
    return nullptr;
}

size_t Canvas::getLayerIndexById(const std::string& id) {
    for (size_t i = 0; i < layers.size(); ++i) {
        if (layers[i]->getId() == id) {
            return i;
        }
    }
    return 0;
}

void Canvas::clear() {
    for (auto& layer : layers) {
        layer->clear();
    }
}

void Canvas::resize(int wWidth, int wHeight) {
    windowWidth = wWidth;
    windowHeight = wHeight;
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

void Canvas::undo() {
    if (commandManager) {
        commandManager->undo();
    }
}

void Canvas::redo() {
    if (commandManager) {
        commandManager->redo();
    }
}

bool Canvas::canUndo() const {
    return commandManager && commandManager->canUndo();
}

bool Canvas::canRedo() const {
    return commandManager && commandManager->canRedo();
}