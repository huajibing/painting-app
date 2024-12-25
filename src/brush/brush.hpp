#pragma once
#include "../utils/color.hpp"
#include <memory>
#include <vector>

class Brush {
public:
    virtual ~Brush() = default;
    virtual void init() = 0;
    
    virtual void drawPoint(float x, float y,
                        float size, const Color& color) = 0;
    virtual std::vector<std::vector<float>> drawLine(
        float x1, float y1,
        float x2, float y2, 
        float size, const Color& color) = 0;

    virtual void beginStroke() = 0;
    virtual void endStroke() = 0;
};