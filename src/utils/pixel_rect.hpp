#pragma once

struct PixelRect {
    int x, y, width, height;
    
    PixelRect(int x = 0, int y = 0, int w = 0, int h = 0)
        : x(x), y(y), width(w), height(h) {}
};