#pragma once
#include <glad/glad.h>
#include "../utils/color.hpp"
#include "../utils/shader.hpp"
#include <memory>
#include <vector>
#include "base_brush.hpp"
#include "texture_brush.hpp"
#include "watercolor_brush.hpp"
#include "oil_paint_brush.hpp"
#include "crayon_brush.hpp"
#include "calligraphy_brush.hpp"

enum class BrushType {
    BaseCircle,
    BaseSquare,
    TexturePencil,
    TextureAcrylic,
    SoftCircle,
    Watercolor,
    Rough,
    OilPaint,
    Crayon,
    // Calligraphy
};

static const std::vector<BrushType> brushTypes = {
    BrushType::BaseCircle,
    BrushType::BaseSquare,
    BrushType::TexturePencil,
    BrushType::TextureAcrylic,
    BrushType::SoftCircle,
    BrushType::Watercolor,
    BrushType::Rough,
    BrushType::OilPaint,
    BrushType::Crayon,
    // BrushType::Calligraphy
};

static std::string getBrushNameByType(BrushType type) {
    switch(type) {
        case BrushType::BaseCircle:
            return "Circle";
        case BrushType::BaseSquare:
            return "Square";
        case BrushType::TexturePencil:
            return "Pencil";
        case BrushType::TextureAcrylic:
            return "Acrylic";
        case BrushType::SoftCircle:
            return "Soft Circle";
        case BrushType::Watercolor:
            return "Watercolor";
        case BrushType::Rough:
            return "Rough";
        case BrushType::OilPaint:
            return "Oil Paint";
        case BrushType::Crayon:
            return "Crayon";
        // case BrushType::Calligraphy:
        //     return "Calligraphy";
        default:
            return "Unknown";
    }
}

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
            case BrushType::SoftCircle:
                return std::make_shared<TextureBrush::SoftCircleBrush>(framebuffer, texture, width, height);
            case BrushType::Watercolor:
                return std::make_shared<WatercolorBrush::WatercolorBrush>(framebuffer, texture, width, height);
            case BrushType::Rough:
                return std::make_shared<OilPaintBrush::RoughBrush>(framebuffer, texture, width, height);
            case BrushType::OilPaint:
                return std::make_shared<OilPaintBrush::OilPaintBrush>(framebuffer, texture, width, height);
            case BrushType::Crayon:
                return std::make_shared<CrayonBrush::CrayonBrush>(framebuffer, texture, width, height);
            // case BrushType::Calligraphy:
            //     return std::make_shared<CalligraphyBrush::CalligraphyBrush>(framebuffer, texture, width, height);
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