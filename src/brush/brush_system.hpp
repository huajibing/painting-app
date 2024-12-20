#pragma once
#include "basic_brush.hpp"
#include "../core/canvas.hpp"
#include "stroke_buffer.hpp"
#include "../commands/draw_commands.hpp"
#include <memory>
#include <vector>

class BrushSystem {
public:
    BrushSystem(Canvas& canvas);
    
    void beginStroke();
    void endStroke();
    void draw(float x, float y);
    void drawPoint(float x, float y);
    void updateBrushSettings(float size, const Color& color);
    void setCommandManager(CommandManager* manager) { commandManager = manager; }
    void disableCommand() { commandEnabled = false; }
    
private:
    Canvas& canvas;
    BasicBrush brush;
    bool lastPos_initialized;
    float lastX, lastY;
    
    std::unique_ptr<StrokeBuffer> strokeBuffer;
    bool isStroking;

    bool commandEnabled;
    CommandManager* commandManager;
    std::vector<StrokePoint> currentStrokePoints;
    std::unique_ptr<StrokeCommand> createStrokeCommand();
};