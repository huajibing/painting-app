#include "brush_system.hpp"
#include <cmath>
#include <iostream>

BrushSystem::BrushSystem(Canvas& canvas) 
    : canvas(canvas), lastPos_initialized(false), lastX(0), lastY(0),
      isStroking(false) {
    strokeBuffer = std::make_unique<StrokeBuffer>(canvas.getWidth(), canvas.getHeight());
    strokeBuffer->init();
}

void BrushSystem::beginStroke() {
    isStroking = true;
    lastPos_initialized = false;
    strokeBuffer->clear();
    canvas.setStroking(true);
    canvas.setStrokeTexture(strokeBuffer->getTexture());
}

void BrushSystem::endStroke() {
    if (isStroking) {
        Layer* activeLayer = canvas.getLayer(canvas.getActiveLayerIndex());
        if (activeLayer) {
            activeLayer->mergeStroke(strokeBuffer->getTexture());
        }
        isStroking = false;
        canvas.setStroking(false);
        canvas.setStrokeTexture(0);
    }
}

void BrushSystem::draw(float x, float y) {
    if (!isStroking) {
        return;
    }
    
    if (!lastPos_initialized) {
        // First point in stroke
        strokeBuffer->drawPoint(x, y, brush.getSize(), brush.getColor());
        lastPos_initialized = true;
    } else {
        // Draw line from last position to current position
        strokeBuffer->drawLine(lastX, lastY, x, y, brush.getSize(), brush.getColor());
    }
    
    lastX = x;
    lastY = y;
}

void BrushSystem::updateBrushSettings(float size, const Color& color) {
    brush.setSize(size);
    brush.setColor(color);
}