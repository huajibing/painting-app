#pragma once
#include "basic_brush.hpp"
#include "../core/canvas.hpp"
#include "stroke_buffer.hpp"
#include <memory>

class BrushSystem {
public:
    BrushSystem(Canvas& canvas);
    
    void beginStroke();
    void endStroke();
    // void resetDrawState() { lastPos_initialized = false; }
    void draw(float x, float y);
    void updateBrushSettings(float size, const Color& color);
    
private:
    Canvas& canvas;
    BasicBrush brush;
    bool lastPos_initialized;
    float lastX, lastY;
    
    std::unique_ptr<StrokeBuffer> strokeBuffer;
    bool isStroking;
};