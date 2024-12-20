#pragma once
#include "command_system.hpp"
#include "../brush/brush_system.hpp"
#include <vector>

// Structure to store stroke data
struct StrokePoint {
    float x, y;
    float pressure;  // For future pressure sensitivity support
    
    StrokePoint(float x, float y, float pressure = 1.0f)
        : x(x), y(y), pressure(pressure) {}
};

// Helper to convert stroke bounds to pixel rectangle
inline PixelRect strokeBoundsToRect(float x, float y, float width, float height) {
    // Add some padding to ensure we capture all affected pixels
    const float padding = 2.0f;  // Extra pixels to account for antialiasing
    return PixelRect(
        static_cast<int>(std::floor(x - padding)),
        static_cast<int>(std::floor(y - padding)),
        static_cast<int>(std::ceil(width + 2 * padding)),
        static_cast<int>(std::ceil(height + 2 * padding))
    );
}

// Command for drawing strokes
class StrokeCommand : public Command {
public:
    StrokeCommand(const std::vector<StrokePoint>& points,
                 float size,
                 const Color& color,
                 size_t layerIndex)
        : points(points), 
          brushSize(size),
          brushColor(color),
          layerIndex(layerIndex) {
        // Calculate affected area bounds
        calculateBounds();
    }
    
    void execute() override {
        if (!canvas) return;
        
        Layer* layer = canvas->getLayer(layerIndex);
        if (!layer) return;

        // Store original pixels before first execution
        if (originalPixels.empty()) {
            try {
                PixelRect rect = strokeBoundsToRect(bounds.x, bounds.y, 
                                                  bounds.width, bounds.height);
                originalPixels = layer->getPixels(rect);
                // Store the rect for later use in undo
                savedRect = rect;
            } catch (const std::exception& e) {
                std::cerr << "Failed to capture original pixels: " << e.what() << std::endl;
                return;
            }
        }
        
        // Set active layer and draw the stroke
        canvas->setActiveLayer(layerIndex);
        
        // Draw the stroke points
        for (size_t i = 1; i < points.size(); ++i) {
            const auto& p1 = points[i - 1];
            const auto& p2 = points[i];
            layer->drawLine(p1.x, p1.y, p2.x, p2.y, brushSize, brushColor);
        }
    }
    
    void undo() override {
        if (!canvas || originalPixels.empty()) return;
        
        Layer* layer = canvas->getLayer(layerIndex);
        if (!layer) return;

        try {
            // Restore the original pixels
            layer->setPixels(savedRect, originalPixels);
        } catch (const std::exception& e) {
            std::cerr << "Failed to restore original pixels: " << e.what() << std::endl;
        }
    }
    
private:
    void calculateBounds() {
        if (points.empty()) return;
        
        // Initialize bounds with first point
        bounds.x = points[0].x;
        bounds.y = points[0].y;
        bounds.width = bounds.height = brushSize;
        
        // Expand bounds to include all points and brush size
        for (const auto& point : points) {
            float x = point.x;
            float y = point.y;
            
            bounds.x = std::min(bounds.x, x - brushSize);
            bounds.y = std::min(bounds.y, y - brushSize);
            bounds.width = std::max(bounds.width, x + brushSize - bounds.x);
            bounds.height = std::max(bounds.height, y + brushSize - bounds.y);
        }
    }
    
    void captureOriginalPixels() {
        // TODO: Implement pixel capture from the affected area
        // This will require adding methods to Layer class for direct pixel access
        // Layer* layer = canvas->getLayer(layerIndex);
        // if (layer) {
        //     originalPixels = layer->getPixels(bounds.x, bounds.y, 
        //                                      bounds.width, bounds.height);
        // }
    }
    
    std::vector<StrokePoint> points;
    float brushSize;
    Color brushColor;
    size_t layerIndex;
    
    struct {
        float x, y, width, height;
    } bounds;
    
    std::vector<float> originalPixels;  // Changed to float for RGBA float format
    PixelRect savedRect;  // Store the actual pixel rectangle used
};