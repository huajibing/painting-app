#pragma once
#include "../utils/color.hpp"
#include <vector>

class BasicBrush {
public:
    BasicBrush() : size(20.0f), color(1.0f, 0.0f, 0.0f, 1.0f) {}
    
    void setSize(float s) { size = s; }
    void setColor(const Color& c) { color = c; }
    
    float getSize() const { return size; }
    const Color& getColor() const { return color; }
    
private:
    float size;
    Color color;
};