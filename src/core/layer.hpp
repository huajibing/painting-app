#pragma once
#include <glad/glad.h>
#include <string>
#include <memory>
#include "../utils/color.hpp"
#include "../utils/shader.hpp"

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
    void render();
    void clear();
    
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
    
    // Drawing operations (similar to current Canvas drawing methods)
    void drawPoint(float x, float y, float size, const Color& color);
    void drawLine(float x1, float y1, float x2, float y2, float size, const Color& color);
    
    void resize(int width, int height);

private:
    void setupBrushBuffers();
    
    std::string name;
    bool isVisible;
    float opacity;
    BlendMode blendMode;
    
    int width;
    int height;
    
    unsigned int frameBuffer;
    unsigned int texture;
    unsigned int brushVAO;
    unsigned int brushVBO;
    
    std::shared_ptr<Shader> brushShader;
};