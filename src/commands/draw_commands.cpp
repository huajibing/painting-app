#include "draw_commands.hpp"
#include "../brush/brush_system.hpp"
#include "../core/canvas.hpp"
#include "../core/layer.hpp"
#include <iostream>

void StrokeCommand::init() {
    if (!canvas) return;
    
    calculateBounds();
    captureOriginalPixels();
}

void StrokeCommand::execute() {
    if (!canvas) return;
    
    Layer* layer = canvas->findLayerById(layerId);
    if (!layer) {
        std::cerr << "Failed to find layer with id: " << layerId << std::endl;
        return;
    }

    // Create brush system for drawing
    std::unique_ptr<BrushSystem> brushSystem = std::make_unique<BrushSystem>(*canvas);
    brushSystem->updateBrushSettings(brushSize, brushColor);
    brushSystem->disableCommand();
    
    // Set active layer and draw the stroke
    size_t layerIndex = 0;
    for (size_t i = 0; i < canvas->getLayerCount(); ++i) {
        if (canvas->getLayer(i)->getId() == layerId) {
            layerIndex = i;
            break;
        }
    }
    canvas->setActiveLayer(layerIndex);
    
    // Draw the stroke points
    brushSystem->beginStroke();
    for (size_t i = 0; i < points.size(); ++i) {
        brushSystem->drawPoint(points[i].x, points[i].y);
    }
    brushSystem->endStroke();
}

void StrokeCommand::undo() {
    if (!canvas || originalPixels.empty()) return;
    
    Layer* layer = canvas->findLayerById(layerId);
    if (!layer) return;

    try {
        // Restore the original pixels
        layer->setPixels(savedRect, originalPixels);
    } catch (const std::exception& e) {
        std::cerr << "Failed to restore original pixels: " << e.what() << std::endl;
    }
}

void StrokeCommand::calculateBounds() {
    if (points.empty()) return;

    const int canvasWidth = canvas->getWidth();
    const int canvasHeight = canvas->getHeight();

    // First find min/max coordinates of all points
    float minX = points[0].x;
    float minY = points[0].y;
    float maxX = points[0].x;
    float maxY = points[0].y;

    for (const auto& point : points) {
        minX = std::min(minX, point.x);
        minY = std::min(minY, point.y);
        maxX = std::max(maxX, point.x);
        maxY = std::max(maxY, point.y);
    }
    
    // Add brush size to create the final bounds
    bounds.x = minX - brushSize;
    bounds.y = minY - brushSize;
    bounds.width = (maxX - minX) + 2 * brushSize;
    bounds.height = (maxY - minY) + 2 * brushSize;

    // Clamp bounds to canvas size
    bounds.width = std::clamp(bounds.x + bounds.width, 0.0f, static_cast<float>(canvasWidth)) 
                    - std::clamp(bounds.x, 0.0f, static_cast<float>(canvasWidth));
    bounds.height = std::clamp(bounds.y + bounds.height, 0.0f, static_cast<float>(canvasHeight))
                    - std::clamp(bounds.y, 0.0f, static_cast<float>(canvasHeight));
    bounds.x = std::clamp(bounds.x, 0.0f, static_cast<float>(canvasWidth));
    bounds.y = std::clamp(bounds.y, 0.0f, static_cast<float>(canvasHeight));
}

void StrokeCommand::captureOriginalPixels() {
    Layer* layer = canvas->findLayerById(layerId);
    if (layer) {
        savedRect = strokeBoundsToRect(bounds.x, bounds.y, 
                                     bounds.width, bounds.height,
                                     canvas->getWidth(), canvas->getHeight());
        originalPixels = layer->getPixels(savedRect);
    }
}