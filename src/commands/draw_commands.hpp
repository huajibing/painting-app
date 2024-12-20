#pragma once
#include "command_system.hpp"
#include "../utils/color.hpp"
#include "../utils/pixel_rect.hpp"
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

// Forward declarations
class BrushSystem;

// Structure to store stroke data
struct StrokePoint {
    float x, y;
    float pressure;
    
    StrokePoint(float x, float y, float pressure = 1.0f)
        : x(x), y(y), pressure(pressure) {}
};

// Helper to convert stroke bounds to pixel rectangle
inline PixelRect strokeBoundsToRect(float x, float y, float width, float height, int canvasWidth, int canvasHeight) {
    const float padding = 2.0f;
    int rectX = static_cast<int>(std::floor(x - padding));
    int rectY = static_cast<int>(std::floor(y - padding));
    int rectW = static_cast<int>(std::ceil(width + 2 * padding));
    int rectH = static_cast<int>(std::ceil(height + 2 * padding));

    rectW = std::clamp(rectX + rectW, 0, canvasWidth - 1) - std::clamp(rectX, 0, canvasWidth - 1);
    rectH = std::clamp(rectY + rectH, 0, canvasHeight - 1) - std::clamp(rectY, 0, canvasHeight - 1);
    rectX = std::clamp(rectX, 0, canvasWidth - 1);
    rectY = std::clamp(rectY, 0, canvasHeight - 1);

    // Invert Y axis, as OpenGL has origin at bottom-left :(
    rectY = canvasHeight - (rectY + rectH);

    std::cout << "strokeBoundsToRect" << std::endl;
    std::cout << "rectX: " << rectX << std::endl;
    std::cout << "rectY: " << rectY << std::endl;
    std::cout << "rectW: " << rectW << std::endl;
    std::cout << "rectH: " << rectH << std::endl;
    
    return PixelRect(rectX, rectY, rectW, rectH);
}

// Command for drawing strokes
class StrokeCommand : public Command {
public:
    StrokeCommand(const std::vector<StrokePoint>& points,
                 float size,
                 const Color& color,
                 const std::string& layerId)
        : points(points), 
          brushSize(size),
          brushColor(color),
          layerId(layerId) {
        std::cout << "StrokeCommand created" << std::endl;
    }
    
    void init() override;
    void execute() override;
    void undo() override;
    
private:
    void calculateBounds();
    void captureOriginalPixels();
    
    std::vector<StrokePoint> points;
    float brushSize;
    Color brushColor;
    std::string layerId;
    
    struct {
        float x, y, width, height;
    } bounds;
    
    std::vector<float> originalPixels;
    PixelRect savedRect;
};