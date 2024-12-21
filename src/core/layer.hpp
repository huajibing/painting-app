#pragma once
#include <glad/glad.h>
#include <string>
#include <memory>
#include <vector>
#include <atomic>
#include "../utils/color.hpp"
#include "../utils/shader.hpp"
#include "../utils/pixel_rect.hpp"
#include "../commands/command_system.hpp"

enum class BlendMode {
    Normal,
    Multiply,
    Screen,
    Overlay
};

class Layer {
public:
    Layer(int width, int height, const std::string& name = "New Layer", CommandManager* manager = nullptr);
    ~Layer();
    
    bool init();
    void clear(); // This action will not be added to the command stack
    void manualClear();
    void setCommandManager(CommandManager* manager) { commandManager = manager; }

    // Generate unique ID
    static std::string generateId() {
        uint64_t currentId = nextId.fetch_add(1, std::memory_order_relaxed);
        return "layer_" + std::to_string(currentId);
    }
    void setId(const std::string& newId) { id = newId; }
    
    // Basic operations
    void setVisibility(bool visible, bool addToCommandStack = true);
    void setOpacity(float value);
    void setBlendMode(BlendMode mode, bool addToCommandStack = true);
    void setName(const std::string& newName, bool addToCommandStack = true);
    
    // Getters
    bool getVisibility() const { return isVisible; }
    float getOpacity() const { return opacity; }
    BlendMode getBlendMode() const { return blendMode; }
    const std::string& getName() const { return name; }
    unsigned int getTexture() const { return texture; }
    const std::string& getId() const { return id; }
    
    void mergeStroke(unsigned int strokeTexture);
    
    void resize(int width, int height);

    // Pixel access methods
    std::vector<float> getPixels(const PixelRect& rect) const;
    void setPixels(const PixelRect& rect, const std::vector<float>& pixels);
    
    // Helper method to validate rectangle bounds
    bool validateRect(const PixelRect& rect) const;

private:
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

    CommandManager* commandManager;
};