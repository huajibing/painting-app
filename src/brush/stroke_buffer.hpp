#pragma once
#include <glad/glad.h>
#include "../utils/color.hpp"
#include "../utils/shader.hpp"
#include <memory>
#include <vector>
#include "base_brush.hpp"
#include "texture_brush.hpp"

enum class BrushType {
    BaseCircle,
    BaseSquare,
    TexturePencil,
    TextureAcrylic
};

class BrushFactory {
public:
    static std::shared_ptr<Brush> createBrush(
        BrushType type,
        unsigned int framebuffer,
        unsigned int texture,
        int width, 
        int height
    ) {
        switch(type) {
            case BrushType::BaseCircle:
                return std::make_shared<BaseBrush::CircleBrush>(framebuffer, width, height);
            case BrushType::BaseSquare:
                return std::make_shared<BaseBrush::SquareBrush>(framebuffer, width, height);
            case BrushType::TexturePencil:
                return std::make_shared<TextureBrush::PencilBrush>(framebuffer, texture, width, height);
            case BrushType::TextureAcrylic:
                return std::make_shared<TextureBrush::AcrylicBrush>(framebuffer, texture, width, height);
            default:
                return std::make_shared<BaseBrush::CircleBrush>(framebuffer, width, height);
        }
    }
};

class StrokeBuffer {
public:
    StrokeBuffer(int width, int height);
    ~StrokeBuffer();
    
    bool init();
    void clear();
    
    // Drawing operations
    void drawPoint(float x, float y, float size, const Color& color);
    std::vector<std::vector<float>> drawLine(float x1, float y1, float x2, float y2, float size, const Color& color);
    void beginStroke(BrushType type);
    void endStroke();
    
    // Get the texture containing the stroke
    unsigned int getTexture() const { return texture; }
    
private:    
    int width;
    int height;
    
    unsigned int frameBuffer;
    unsigned int texture;
    unsigned int brushVAO;
    unsigned int brushVBO;

    std::shared_ptr<Brush> brush;
};