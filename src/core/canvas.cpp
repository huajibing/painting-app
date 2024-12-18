#include "canvas.hpp"
#include <glad/glad.h>
#include <iostream>
#include <cmath>
#include "../utils/shaders.hpp"

Canvas::Canvas(int w, int h) 
    : textureWidth(w), textureHeight(h), 
      windowWidth(0), windowHeight(0),
      backgroundColor(Color(1.0f, 1.0f, 1.0f, 1.0f)),
      projection(1.0f), canvasScale(0.8f),
      frameBuffer(0), texture(0), 
      VAO(0), VBO(0), EBO(0),
      brushVAO(0), brushVBO(0) {
}

void Canvas::saveViewport() {
    glGetIntegerv(GL_VIEWPORT, savedViewport);
}

void Canvas::restoreViewport() {
    glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);
}

bool Canvas::init() {
    try {
        // setup shaders
        shader = std::make_unique<Shader>(Shaders::canvasVertexShader, 
                                        Shaders::canvasFragmentShader);
        brushShader = std::make_unique<Shader>(Shaders::brushVertexShader, 
                                             Shaders::brushFragmentShader);
        
        // setup framebuffer
        glGenFramebuffers(1, &frameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        
        // setup texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                              GL_TEXTURE_2D, texture, 0);
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer is not complete!" << std::endl;
            return false;
        }
        
        setupQuad();
        setupBrushBuffers();
        clear();
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    
    // Clear the canvas area to white
    glEnable(GL_SCISSOR_TEST);
    int canvasX, canvasY, canvasWidth, canvasHeight;
    getCanvasRect(canvasX, canvasY, canvasWidth, canvasHeight);
    glScissor(canvasX, canvasY, canvasWidth, canvasHeight);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // White background
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
    
    shader->use();
    glUniformMatrix4fv(glGetUniformLocation(shader->getProgram(), "projection"), 
                       1, GL_FALSE, &projection[0][0]);
    
    // Setup proper blending for the canvas
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    restoreViewport();
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
    
    // Create orthographic projection matrix
    projection = glm::mat4(1.0f);
    projection = glm::scale(projection, glm::vec3(scaleX, scaleY, 1.0f));

    std::cout << "Canvas updated - Window: " << windowWidth << "x" << windowHeight 
              << ", Display: " << displayWidth << "x" << displayHeight
              << ", Texture: " << textureWidth << "x" << textureHeight << std::endl;
}

void Canvas::resize(int wWidth, int wHeight) {
    windowWidth = wWidth;
    windowHeight = wHeight;

    updateProjection();
}

bool Canvas::windowToCanvas(double windowX, double windowY, 
                          float& canvasX, float& canvasY) const {
    // Compute canvas position based on window coordinates
    float canvasLeft = (windowWidth - displayWidth) * 0.5f;
    float canvasTop = (windowHeight - displayHeight) * 0.5f;
    
    // Check if point is inside canvas
    if (windowX >= canvasLeft && windowX < canvasLeft + displayWidth &&
        windowY >= canvasTop && windowY < canvasTop + displayHeight) {
        
        // Convert window coordinates to canvas coordinates
        canvasX = (windowX - canvasLeft) / displayWidth * textureWidth;
        canvasY = (windowY - canvasTop) / displayHeight * textureHeight;
        return true;
    }
    return false;
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

Canvas::~Canvas() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &brushVAO);
    glDeleteBuffers(1, &brushVBO);
    glDeleteFramebuffers(1, &frameBuffer);
    glDeleteTextures(1, &texture);
}

void Canvas::clear() {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, textureWidth, textureHeight);
    glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Canvas::setupBrushBuffers() {
    std::vector<float> vertices;
    const int segments = 32;
    const float radius = 1.0f;
    
    // Center point
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    
    // Circle vertices
    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * 3.14159f * float(i) / float(segments);
        float x = radius * cosf(theta);
        float y = radius * sinf(theta);
        vertices.push_back(x);
        vertices.push_back(y);
    }
    
    glGenVertexArrays(1, &brushVAO);
    glGenBuffers(1, &brushVBO);
    
    glBindVertexArray(brushVAO);
    glBindBuffer(GL_ARRAY_BUFFER, brushVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Canvas::drawLine(float x1, float y1, float x2, float y2, float size, const Color& color) {
    // std::cout << "===Drawing line from " << x1 << ", " << y1 << " to " << x2 << ", " << y2 << std::endl;

    // Compute distance between points
    float dx = x2 - x1;
    float dy = y2 - y1;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    // Determine number of steps based on distance
    int steps = std::max(2, int(distance * 2));  // At least 2 steps
    
    // Draw points along line
    for (int i = 0; i < steps; ++i) {
        float t = float(i) / (steps - 1);
        float x = x1 + dx * t;
        float y = y1 + dy * t;
        drawPoint(x, y, size, color);
    }
}

void Canvas::drawPoint(float x, float y, float size, const Color& color) {
    saveViewport();
    
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    // std::cout << "Binding to FBO: " << frameBuffer << std::endl;
    
    glViewport(0, 0, textureWidth, textureHeight);
    
    brushShader->use();
    
    // Convert canvas coordinates to clip space
    float clipX = (x / textureWidth) * 2.0f - 1.0f;
    float clipY = 1.0f - (y / textureHeight) * 2.0f;

    // // debug
    // std::cout << "Draw parameters:" << std::endl;
    // std::cout << "Position: " << x << ", " << y << std::endl;
    // std::cout << "Clip space: " << clipX << ", " << clipY << std::endl;
    // std::cout << "Size: " << size << std::endl;
    // std::cout << "Color: " << color.r << ", " << color.g << ", " 
    //           << color.b << ", " << color.a << std::endl;
    
    float aspectRatio = 1.0f / (static_cast<float>(textureWidth) / textureHeight);
    glUniform1f(glGetUniformLocation(brushShader->getProgram(), "aspectRatio"), aspectRatio);
    
    glUniform2f(glGetUniformLocation(brushShader->getProgram(), "position"), clipX, clipY);
    
    float normalizedSize = size / static_cast<float>(textureHeight);
    glUniform1f(glGetUniformLocation(brushShader->getProgram(), "size"), normalizedSize);
    
    glUniform4f(glGetUniformLocation(brushShader->getProgram(), "brushColor"), 
                color.r, color.g, color.b, color.a);
    
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 
                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    
    glBindVertexArray(brushVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 34);
    
    // Check for OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in drawPoint: " << err << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    restoreViewport();
}