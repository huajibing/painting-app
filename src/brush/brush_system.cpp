#include "brush_system.hpp"
#include <cmath>
#include <iostream>

BrushSystem::BrushSystem(Canvas& canvas) 
    : canvas(canvas), lastPos_initialized(false), lastX(0), lastY(0) {}

void BrushSystem::draw(float x, float y) {
    float texX = x;
    float texY = y;
    
    if (!lastPos_initialized) {
        // First point in stroke
        canvas.drawPoint(texX, texY, brush.getSize(), brush.getColor());
        lastPos_initialized = true;
    } else {
        // Draw line from last position to current position
        canvas.drawLine(lastX, lastY, texX, texY, brush.getSize(), brush.getColor());
    }
    
    lastX = texX;
    lastY = texY;
    
    // // Debug output
    // std::cout << "Drawing at: " << texX << ", " << texY << std::endl;
    // std::cout << "Brush size: " << brush.getSize() << std::endl;
    // std::cout << "Brush color: " << brush.getColor().r << ", " << brush.getColor().g << ", " << brush.getColor().b << std::endl;
}

void BrushSystem::updateBrushSettings(float size, const Color& color) {
    brush.setSize(size);
    brush.setColor(color);
}