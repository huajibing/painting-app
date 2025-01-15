#include "layer.hpp"
#include "../utils/shaders.hpp"
#include "../commands/layer_commands.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <algorithm>

std::atomic<uint64_t> Layer::nextId{0};

Layer::Layer(int w, int h, const std::string& layerName, CommandManager* manager)
    : name(layerName), isVisible(true), opacity(1.0f),
      blendMode(BlendMode::Normal), width(w), height(h),
      frameBuffer(0), texture(0), id(generateId()),
      commandManager(manager) {
}

Layer::~Layer() {
    glDeleteFramebuffers(1, &frameBuffer);
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &mergeVAO);
    glDeleteBuffers(1, &mergeVBO);
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
    
    clear();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Initialize merge shader
    mergeShader = std::make_shared<Shader>(Shaders::mergeVertexShader, 
                                         Shaders::mergeFragmentShader);
    
    // Setup quad for merging
    float quadVertices[] = {
        // positions        // texture coords
        -1.0f,  1.0f,     0.0f, 1.0f,
        -1.0f, -1.0f,     0.0f, 0.0f,
         1.0f,  1.0f,     1.0f, 1.0f,
         1.0f, -1.0f,     1.0f, 0.0f
    };
    
    glGenVertexArrays(1, &mergeVAO);
    glGenBuffers(1, &mergeVBO);
    
    glBindVertexArray(mergeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mergeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    // Texture coord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    return true;
}

void Layer::setVisibility(bool visible, bool addToCommandStack) {
    // Create command for modifying layer visibility
    if (commandManager && addToCommandStack) {
        auto command = std::make_unique<ModifyLayerCommand>(id, ModifyLayerCommand::PropertyType::Visibility, visible);
        commandManager->addCommand(std::move(command));
    }
    isVisible = visible;
}

void Layer::setOpacity(float value) {
    opacity = std::clamp(value, 0.0f, 1.0f);
}

void Layer::setBlendMode(BlendMode mode, bool addToCommandStack) {
    // Create command for modifying layer blend mode
    if (commandManager && addToCommandStack) {
        auto command = std::make_unique<ModifyLayerCommand>(id, ModifyLayerCommand::PropertyType::BlendMode, mode);
        commandManager->addCommand(std::move(command));
    }
    blendMode = mode;
}

void Layer::setName(const std::string& newName, bool addToCommandStack) {
    // Create command for modifying layer name
    if (commandManager && addToCommandStack) {
        auto command = std::make_unique<ModifyLayerCommand>(id, ModifyLayerCommand::PropertyType::Name, newName);
        commandManager->addCommand(std::move(command));
    }
    name = newName;
}

void Layer::clear() {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Layer::manualClear() {
    // Create command for clearing the layer
    if (commandManager) {
        auto command = std::make_unique<ClearLayerCommand>(id);
        commandManager->addCommand(std::move(command));
    }
    clear();
}

void Layer::mergeStroke(unsigned int strokeTexture, StrokeMode mode) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, width, height);
    
    // Use merge shader
    mergeShader->use();
    
    // Set layer texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(mergeShader->getProgram(), "layerTexture"), 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, strokeTexture);
    glUniform1i(glGetUniformLocation(mergeShader->getProgram(), "strokeTexture"), 1);

    glUniform1i(glGetUniformLocation(mergeShader->getProgram(), "strokeMode"), 
                mode == StrokeMode::Erase ? 1 : 0);
    
    // Disable OpenGL blending
    glDisable(GL_BLEND);
    
    // Draw full screen quad
    glBindVertexArray(mergeVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    // Cleanup
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Unused
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

bool Layer::validateRect(const PixelRect& rect) const {
    // Check if rectangle is within layer bounds
    if (rect.x < 0 || rect.y < 0 || 
        rect.width <= 0 || rect.height <= 0 ||
        rect.x + rect.width > width ||
        rect.y + rect.height > height) {
        return false;
    }
    return true;
}

std::vector<float> Layer::getPixels(const PixelRect& rect) const {
    if (!validateRect(rect)) {
        throw std::invalid_argument("Invalid pixel rectangle");
    }
    
    // Calculate buffer size (4 channels: RGBA)
    size_t bufferSize = rect.width * rect.height * 4;
    std::vector<float> pixels(bufferSize);
    
    // Bind framebuffer to read from
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    // Read pixels from the specified rectangle
    glReadPixels(rect.x, rect.y, rect.width, rect.height, 
                 GL_RGBA, GL_FLOAT, pixels.data());
    
    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return pixels;
}

void Layer::setPixels(const PixelRect& rect, const std::vector<float>& pixels) {
    if (!validateRect(rect)) {
        throw std::invalid_argument("Invalid pixel rectangle");
    }
    
    // Verify buffer size
    size_t expectedSize = rect.width * rect.height * 4;
    if (pixels.size() != expectedSize) {
        throw std::invalid_argument("Invalid pixel buffer size");
    }
    
    // Bind framebuffer and texture
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Store current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    // Set viewport to match layer dimensions
    glViewport(0, 0, width, height);
    
    // Update the texture data for the specified rectangle
    glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, 
                    rect.width, rect.height, GL_RGBA, GL_FLOAT, 
                    pixels.data());
    
    // Restore viewport
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    
    // Unbind framebuffer and texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}