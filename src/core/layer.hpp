#pragma once
#include <glad/glad.h>
#include <string>
#include <memory>
#include <vector>
#include <atomic>
#include "../utils/color.hpp"
#include "../utils/shader.hpp"
#include "../utils/pixel_rect.hpp"

enum class BlendMode {
    Normal,
    Multiply,
    Screen,
    Overlay
};

class Layer {
public:
    Layer(int width, int height, const std::string& name = "New Layer");
    ~Layer();
    
    bool init();
    void clear();

    // Generate unique ID
    static std::string generateId() {
        uint64_t currentId = nextId.fetch_add(1, std::memory_order_relaxed);
        return "layer_" + std::to_string(currentId);
    }
    const std::string& getId() const { return id; }
    
    // Basic operations
    void setVisibility(bool visible) { isVisible = visible; }
    void setOpacity(float value) { opacity = value; }
    void setBlendMode(BlendMode mode) { blendMode = mode; }
    void setName(const std::string& newName) { name = newName; }
    
    // Getters
    bool getVisibility() const { return isVisible; }
    float getOpacity() const { return opacity; }
    BlendMode getBlendMode() const { return blendMode; }
    const std::string& getName() const { return name; }
    unsigned int getTexture() const { return texture; }
    
    void mergeStroke(unsigned int strokeTexture);
    
    void resize(int width, int height);

    // Pixel access methods
    std::vector<float> getPixels(const PixelRect& rect) const;
    void setPixels(const PixelRect& rect, const std::vector<float>& pixels);
    
    // Helper method to validate rectangle bounds
    bool validateRect(const PixelRect& rect) const;

private:
    void setupBrushBuffers();
    
    static std::atomic<uint64_t> nextId;
    std::string id;
    std::string name;
    bool isVisible;
    float opacity;
    BlendMode blendMode;
    
    int width;
    int height;
    
    unsigned int frameBuffer;
    unsigned int texture;
    unsigned int mergeVAO;
    unsigned int mergeVBO;

    std::shared_ptr<Shader> mergeShader;

    mutable std::vector<float> pixelBuffer;
};