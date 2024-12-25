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

void StrokeBuffer::drawPoint(float x, float y, float size, const Color& color) {
    brush->drawPoint(x, y, size, color);
}

std::vector<std::vector<float>> StrokeBuffer::drawLine(float x1, float y1, float x2, float y2, float size, const Color& color) {
    std::vector<std::vector<float>> points = brush->drawLine(x1, y1, x2, y2, size, color);
    return points;
}

void StrokeBuffer::beginStroke(BrushType type) {
    brush = BrushFactory::createBrush(type, frameBuffer, width, height);
    brush->init();
    brush->beginStroke();
}

void StrokeBuffer::endStroke() {
    brush->endStroke();
}