#include "base_brush.hpp"
#include <cmath>
#include <iostream>
#define PI 3.14159265359

namespace BaseBrush {

    BaseBrush::BaseBrush(unsigned int framebuffer, int width, int height)
        : frameBuffer(framebuffer), width(width), height(height) {
    }

    BaseBrush::~BaseBrush() {
        glDeleteVertexArrays(1, &brushVAO);
        glDeleteBuffers(1, &brushVBO);
    }

    void BaseBrush::init() {
        // Create brush shader
        brushShader = std::make_shared<Shader>(baseVertexShader, baseFragmentShader);
        
        // Setup brush buffers
        glGenVertexArrays(1, &brushVAO);
        glGenBuffers(1, &brushVBO);
        
        glBindVertexArray(brushVAO);
        glBindBuffer(GL_ARRAY_BUFFER, brushVBO);
        
        // Set up vertex attributes
        std::vector<float> vertices = getVertices();
        vertexCount = vertices.size() / 2;
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void BaseBrush::drawPoint(float x, float y, float size, const Color& color) {
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
        
        // Disable blending
        glDisable(GL_BLEND);
        
        // Draw brush
        glBindVertexArray(brushVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    std::vector<std::vector<float>> BaseBrush::drawLine(float x1, float y1, float x2, float y2, float size, const Color& color) {
        float dx = x2 - x1;
        float dy = y2 - y1;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        int steps = std::max(2, int(distance * 2));

        std::vector<std::vector<float>> points;
        
        for (int i = 0; i < steps; ++i) {
            float t = float(i) / (steps - 1);
            float x = x1 + dx * t;
            float y = y1 + dy * t;
            drawPoint(x, y, size, color);
            points.push_back({x, y});
        }
        
        return points;
    }

    std::vector<float> CircleBrush::getVertices() {
        std::vector<float> vertices;
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        
        for (int i = 0; i <= 32; ++i) {
            float angle = i * 2 * PI / 32.0f;
            vertices.push_back(std::cos(angle));
            vertices.push_back(std::sin(angle));
        }
        
        return vertices;
    }

    std::vector<float> SquareBrush::getVertices() {
        return {
            0.0f, 0.0f,
            -1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, -1.0f,
            -1.0f, -1.0f
        };
    }

} // namespace BaseBrush