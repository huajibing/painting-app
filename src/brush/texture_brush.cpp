#include "texture_brush.hpp"
#include <iostream>
#include <cmath>
#include "stb_image.h"
#define PI 3.14159265359

namespace TextureBrush {

    TextureBrush::TextureBrush(unsigned int framebuffer, unsigned int texture, int w, int h)
        : frameBuffer(framebuffer), texture(texture), width(w), height(h),
        brushVAO(0), brushVBO(0), brushTexture(0) {
        // Initialize random number generator
        std::random_device rd;
        rng = std::mt19937(rd());
        rotationDist = std::uniform_real_distribution<float>(0.0f, 1.0f);
        scatterDist = std::uniform_real_distribution<float>(-1.0f, 1.0f);
    }

    TextureBrush::~TextureBrush() {
        glDeleteVertexArrays(1, &brushVAO);
        glDeleteBuffers(1, &brushVBO);
        glDeleteTextures(1, &brushTexture);
    }

    void TextureBrush::init() {
        // Create shader
        brushShader = std::make_shared<Shader>(textureVertexShader, textureFragmentShader);
        
        // Setup brush buffers
        glGenVertexArrays(1, &brushVAO);
        glGenBuffers(1, &brushVBO);
        
        glBindVertexArray(brushVAO);
        glBindBuffer(GL_ARRAY_BUFFER, brushVBO);
        
        // Set up vertex attributes
        std::vector<float> vertices = getVertices();
        vertexCount = vertices.size() / 4;
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    std::vector<float> TextureBrush::getVertices() {
        std::vector<float> vertices;
        // pos(2) + texCoord(2)
        vertices.push_back(0.0f); // pos x
        vertices.push_back(0.0f); // pos y
        vertices.push_back(0.5f); // tex u 
        vertices.push_back(0.5f); // tex v
        
        for (int i = 0; i <= 32; ++i) {
            float angle = i * 2 * PI / 32.0f;
            float x = std::cos(angle);
            float y = std::sin(angle);
            vertices.push_back(x);    // pos x
            vertices.push_back(y);    // pos y
            vertices.push_back(x * 0.5f + 0.5f); // tex u
            vertices.push_back(y * 0.5f + 0.5f); // tex v  
        }
        return vertices;
    }

    void TextureBrush::loadTexture(const std::string& path) {
        if (brushTexture) {
            glDeleteTextures(1, &brushTexture);
        }
        
        int texWidth, texHeight, nrChannels;
        unsigned char* data = stbi_load(path.c_str(), &texWidth, &texHeight, &nrChannels, 0);
        
        if (data) {
            glGenTextures(1, &brushTexture);
            glBindTexture(GL_TEXTURE_2D, brushTexture);
            
            // Set texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format, 
                        GL_UNSIGNED_BYTE, data);
            
            stbi_image_free(data);
        } else {
            std::cerr << "Failed to load texture: " << path << std::endl;
        }
    }

    void TextureBrush::drawPoint(float x, float y, float size, const Color& color) {
        if (!isDrawing) {
            isDrawing = true;
            lastX = x;
            lastY = y;
        }

        float distance = std::sqrt((x - lastX) * (x - lastX) + (y - lastY) * (y - lastY));
        if (distance < minDistance * size) {
            return;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glViewport(0, 0, width, height);
        
        brushShader->use();
        
        // Convert coordinates to clip space
        float clipX = (x / width) * 2.0f - 1.0f;
        float clipY = -(y / height) * 2.0f + 1.0f;
        
        // Apply random variations
        float rotation = getRandomRotation();
        glm::vec2 scatter = getRandomScatter();
        float sizeMultiplier = getRandomSizeMultiplier();

        // Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, brushTexture);
        brushShader->setInt("brushTexture", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture);
        brushShader->setInt("layerTexture", 1);

        // Set uniforms
        float aspectRatio = static_cast<float>(height) / width;
        brushShader->setFloat("aspectRatio", aspectRatio);
        brushShader->setVec2("position", glm::vec2(clipX, clipY));
        brushShader->setFloat("rotation", rotation);
        brushShader->setFloat("size", (size / height) * 2.0f);
        brushShader->setVec4("brushColor", glm::vec4(color.r, color.g, color.b, color.a));

        // Disable blend
        glDisable(GL_BLEND);
        
        // Draw brush
        glBindVertexArray(brushVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        lastX = x;
        lastY = y;
    }

    std::vector<std::vector<float>> TextureBrush::drawLine(float x1, float y1, 
                                                        float x2, float y2,
                                                        float size, const Color& color) {
        float dx = x2 - x1;
        float dy = y2 - y1;
        float distance = std::sqrt(dx * dx + dy * dy);

        size *= sizeScale;
        
        // Calculate number of stamps based on spacing and size
        float stampDistance = size * spacing;
        int stamps = std::max(2, static_cast<int>(distance / stampDistance));

        std::vector<std::vector<float>> points;
        points.reserve(stamps);
        
        for (int i = 0; i < stamps; ++i) {
            float t = static_cast<float>(i) / (stamps - 1);
            float x = x1 + dx * t;
            float y = y1 + dy * t;
            
            drawPoint(x, y, size, color);
            points.push_back({x, y});
        }

        return points;
    }

    float TextureBrush::getRandomRotation() {
        float baseRotation = minRotation + 
                            (maxRotation - minRotation) * rotationDist(rng);
        if (rotationJitter > 0.0f) {
            float jitter = (rotationDist(rng) * 2.0f - 1.0f) * rotationJitter;
            baseRotation += jitter;
        }
        return baseRotation;
    }

    glm::vec2 TextureBrush::getRandomScatter() {
        if (minScatter == 0.0f && maxScatter == 0.0f) {
            return glm::vec2(0.0f);
        }
        
        float scatter = minScatter + (maxScatter - minScatter) * 
                        std::abs(scatterDist(rng));
        float angle = rotationDist(rng) * 360.0f;
        
        float rad = glm::radians(angle);
        return glm::vec2(
            std::cos(rad) * scatter / width,
            std::sin(rad) * scatter / height
        );
    }

    float TextureBrush::getRandomSizeMultiplier() {
        if (sizeJitter <= 0.0f) return 1.0f;
        return 1.0f + (scatterDist(rng) * sizeJitter);
    }

    void TextureBrush::beginStroke() {
        // Reset random generators for consistent stroke start
        rng.seed(std::random_device()());
    }

    void TextureBrush::endStroke() {
        // Clean up any stroke-specific resources if needed
        isDrawing = false;
    }

} // namespace TextureBrush