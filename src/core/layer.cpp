#include "layer.hpp"
#include "../utils/shaders.hpp"
#include <iostream>
#include <vector>
#include <cmath>

Layer::Layer(int w, int h, const std::string& layerName)
    : name(layerName), isVisible(true), opacity(1.0f),
      blendMode(BlendMode::Normal), width(w), height(h),
      frameBuffer(0), texture(0), brushVAO(0), brushVBO(0) {
}

Layer::~Layer() {
    glDeleteFramebuffers(1, &frameBuffer);
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &brushVAO);
    glDeleteBuffers(1, &brushVBO);
}

bool Layer::init() {
    // Create framebuffer
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    // Create texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Layer framebuffer is not complete!" << std::endl;
        return false;
    }
    
    // Initialize brush shader
    brushShader = std::make_shared<Shader>(Shaders::brushVertexShader, 
                                         Shaders::brushFragmentShader);
    
    setupBrushBuffers();
    clear();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void Layer::clear() {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Layer::setupBrushBuffers() {
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
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), 
                 vertices.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
}

void Layer::drawPoint(float x, float y, float size, const Color& color) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, width, height);
    
    brushShader->use();
    
    // Convert coordinates to clip space
    float clipX = (x / width) * 2.0f - 1.0f;
    float clipY = 1.0f - (y / height) * 2.0f;
    
    float aspectRatio = 1.0f / (static_cast<float>(width) / height);
    glUniform1f(glGetUniformLocation(brushShader->getProgram(), "aspectRatio"), aspectRatio);
    glUniform2f(glGetUniformLocation(brushShader->getProgram(), "position"), clipX, clipY);
    
    float normalizedSize = size / static_cast<float>(height);
    glUniform1f(glGetUniformLocation(brushShader->getProgram(), "size"), normalizedSize);
    
    // Adjust color alpha by layer opacity
    Color adjustedColor = color;
    adjustedColor.a *= opacity;
    glUniform4f(glGetUniformLocation(brushShader->getProgram(), "brushColor"),
                adjustedColor.r, adjustedColor.g, adjustedColor.b, adjustedColor.a);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(brushShader->getProgram(), "layerTexture"), 0);
    
    glDisable(GL_BLEND);
        
    glBindVertexArray(brushVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 34);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Layer::drawLine(float x1, float y1, float x2, float y2, float size, const Color& color) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    int steps = std::max(2, int(distance * 2));
    
    for (int i = 0; i < steps; ++i) {
        float t = float(i) / (steps - 1);
        float x = x1 + dx * t;
        float y = y1 + dy * t;
        drawPoint(x, y, size, color);
    }
}

void Layer::resize(int newWidth, int newHeight) {
    // Store old texture
    GLuint oldTexture = texture;
    
    // Create new texture with new size
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, newWidth, newHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Update framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    
    // TODO: Implement content scaling/copying from old texture to new texture
    
    // Update dimensions
    width = newWidth;
    height = newHeight;
    
    // Cleanup
    glDeleteTextures(1, &oldTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}