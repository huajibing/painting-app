#pragma once
#include "basic_brush.hpp"
#include "../core/canvas.hpp"
#include <memory>

class BrushSystem {
public:
    BrushSystem(Canvas& canvas);
    
    void resetDrawState() { lastPos_initialized = false; }
    void draw(float x, float y);
    void setBrushSize(float size) { brush.setSize(size); }
    void setBrushColor(const Color& color) { brush.setColor(color); }
    void updateBrushSettings(float size, const Color& color);
    
private:
    Canvas& canvas;
    BasicBrush brush;
    bool lastPos_initialized;
    float lastX, lastY;
};