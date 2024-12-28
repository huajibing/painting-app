#include "brush_system.hpp"
#include <cmath>
#include <iostream>

BrushSystem::BrushSystem(Canvas& canvas) 
    : canvas(canvas), lastPos_initialized(false), lastX(0), lastY(0),
      isStroking(false), commandManager(nullptr), commandEnabled(true) {
    strokeBuffer = std::make_unique<StrokeBuffer>(canvas.getWidth(), canvas.getHeight());
    strokeBuffer->init();
    setBrushType(BrushType::TextureAcrylic);
}

void BrushSystem::beginStroke() {
    isStroking = true;
    lastPos_initialized = false;
    strokeBuffer->clear();
    strokeBuffer->beginStroke(brushSettings.type);
    canvas.setStroking(true);
    canvas.setStrokeTexture(strokeBuffer->getTexture());
    currentStrokePoints.clear();
}

void BrushSystem::endStroke() {
    if (isStroking) {
        if (commandEnabled && !currentStrokePoints.empty() && commandManager) {
            auto command = createStrokeCommand();
            command->setCanvas(&canvas);
            command->captureOriginalPixels();

            Layer* activeLayer = canvas.getLayer(canvas.getActiveLayerIndex());
            if (activeLayer) {
                activeLayer->mergeStroke(strokeBuffer->getTexture());
            }
            
            isStroking = false;
            canvas.setStroking(false);
            canvas.setStrokeTexture(0);
            currentStrokePoints.clear();
            strokeBuffer->endStroke();

            command->captureSavedPixels();
            commandManager->addCommand(std::move(command));
        }
    }
}

void BrushSystem::draw(float x, float y) {
    if (!isStroking) {
        return;
    }
    
    if (!lastPos_initialized) {
        // First point in stroke
        strokeBuffer->drawPoint(x, y, brushSettings.size, brushSettings.color);
        lastPos_initialized = true;
        StrokePoint point(x, y);
        currentStrokePoints.push_back(point);
    } else {
        // Draw line from last position to current position
        std::vector<std::vector<float>> points = strokeBuffer->drawLine(lastX, lastY, x, y, brushSettings.size, brushSettings.color);
        for (const auto& point : points) {
            StrokePoint strokePoint(point[0], point[1]);
            currentStrokePoints.push_back(strokePoint);
        }
    }
    
    lastX = x;
    lastY = y;
}

void BrushSystem::updateBrushSettings(float size, const Color& color) {
    brushSettings.size = size;
    brushSettings.color = color;
}

std::unique_ptr<StrokeCommand> BrushSystem::createStrokeCommand() {
    Layer* activeLayer = canvas.getLayer(canvas.getActiveLayerIndex());
    if (!activeLayer) {
        return nullptr;
    }

    return std::make_unique<StrokeCommand>(
        currentStrokePoints,
        brushSettings.size,
        brushSettings.color,
        activeLayer->getId()
    );
}