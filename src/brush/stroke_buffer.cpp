#include "stroke_buffer.hpp"
#include "../utils/shaders.hpp"
#include <iostream>
#include <vector>
#include <cmath>

StrokeBuffer::StrokeBuffer(int w, int h)
    : width(w), height(h),
      frameBuffer(0), texture(0),
      brushVAO(0), brushVBO(0) {
}

StrokeBuffer::~StrokeBuffer() {
    glDeleteFramebuffers(1, &frameBuffer);
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &brushVAO);
    glDeleteBuffers(1, &brushVBO);
}

bool StrokeBuffer::init() {
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
        std::cerr << "Stroke buffer framebuffer is not complete!" << std::endl;
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

void StrokeBuffer::clear() {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void StrokeBuffer::setupBrushBuffers() {
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

void StrokeBuffer::drawPoint(float x, float y, float size, const Color& color) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, width, height);
    
    brushShader->use();
    
    // Adjust coordinates to clip space
    float clipX = (x / width) * 2.0f - 1.0f;
    float clipY = -(y / height) * 2.0f + 1.0f;
    
    // Adjust size and aspect ratio
    float aspectRatio = height / static_cast<float>(width);
    brushShader->setFloat("aspectRatio", aspectRatio);
    brushShader->setVec2("position", glm::vec2(clipX, clipY));
    
    // Normalize size
    float normalizedSize = (size / height) * 2.0f;
    brushShader->setFloat("size", normalizedSize);
    
    // Set brush color
    brushShader->setVec4("brushColor", 
                        glm::vec4(color.r, color.g, color.b, color.a));
    
    // Disable OpenGL blending
    glDisable(GL_BLEND);
    
    glBindVertexArray(brushVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 34);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void StrokeBuffer::drawLine(float x1, float y1, float x2, float y2, float size, const Color& color) {
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

void StrokeBuffer::resize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    
    // Recreate texture with new size
    glDeleteTextures(1, &texture);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Update framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    
    clear();
}