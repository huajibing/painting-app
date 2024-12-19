#pragma once
#include <glad/glad.h>
#include "../utils/color.hpp"
#include "../utils/shader.hpp"
#include <memory>

class StrokeBuffer {
public:
    StrokeBuffer(int width, int height);
    ~StrokeBuffer();
    
    bool init();
    void clear();
    void resize(int width, int height);
    
    // Drawing operations
    void drawPoint(float x, float y, float size, const Color& color);
    void drawLine(float x1, float y1, float x2, float y2, float size, const Color& color);
    
    // Get the texture containing the stroke
    unsigned int getTexture() const { return texture; }
    
private:
    void setupBrushBuffers();
    
    int width;
    int height;
    
    unsigned int frameBuffer;
    unsigned int texture;
    unsigned int brushVAO;
    unsigned int brushVBO;
    
    std::shared_ptr<Shader> brushShader;
};