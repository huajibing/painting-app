#pragma once
#include "../core/canvas.hpp"
#include "stroke_buffer.hpp"
#include "../commands/draw_commands.hpp"
#include <memory>
#include <vector>

struct BrushSettings {
    float size = 20.0f;
    Color color = Color(0.0f, 0.0f, 0.0f, 1.0f);
    BrushType type = BrushType::BaseCircle;
}; 

class BrushSystem {
public:
    BrushSystem(Canvas& canvas);
    
    void beginStroke();
    void endStroke();
    void draw(float x, float y);
    void setBrushType(BrushType type) { brushSettings.type = type; };
    void updateBrushSettings(float size, const Color& color);
    void setCommandManager(CommandManager* manager) { commandManager = manager; }
    void disableCommand() { commandEnabled = false; }
    
private:
    Canvas& canvas;
    BrushSettings brushSettings;
    bool lastPos_initialized;
    float lastX, lastY;
    
    std::unique_ptr<StrokeBuffer> strokeBuffer;
    bool isStroking;

    bool commandEnabled;
    CommandManager* commandManager;
    std::vector<StrokePoint> currentStrokePoints;
    std::unique_ptr<StrokeCommand> createStrokeCommand();
};