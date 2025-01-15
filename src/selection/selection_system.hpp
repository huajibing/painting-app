#pragma once
#include <glad/glad.h>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include "../utils/shader.hpp"
#include "../utils/pixel_rect.hpp"
#include "../commands/command_system.hpp"

class Canvas;
class CommandManager;

struct SelectionTransform {
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
};

enum class SelectionMode {
    None,
    Creating,
    Moving,
    Transforming,
};

class SelectionSystem {
public:
    SelectionSystem(Canvas& canvas);
    ~SelectionSystem();

    bool init();
    void render();

    // Selection operations
    void beginSelection(float x, float y);
    void updateSelection(float x, float y);
    void endSelection();
    
    void startMove(float x, float y);
    void updateMove(float x, float y);
    void endMove();

    // Transform operations
    void copySelection();
    void cutSelection();
    void pasteSelection(float x, float y);
    void deleteSelection();
    
    bool hasSelection() const { return selectionMode != SelectionMode::None; }
    PixelRect getSelectionRect() const { return selectionRect; }

private:
    void setupFramebuffer();
    void setupShaders();
    void setupBuffers();
    void renderSelectionOutline();
    
    Canvas& canvas;
    SelectionMode selectionMode;
    PixelRect selectionRect;
    SelectionTransform transform;
    
    // OpenGL objects
    unsigned int frameBuffer;
    unsigned int texture;
    unsigned int VAO, VBO;
    std::shared_ptr<Shader> shader;
    
    float startX, startY;
    float lastX, lastY;
    
    std::vector<float> selectionContent;

    std::vector<float> clipboardContent;
    PixelRect clipboardRect;
    
    CommandManager* commandManager;
};

class SelectionCommand : public Command {
public:
    SelectionCommand(const std::string& layerId, 
                    const PixelRect& rect,
                    const std::vector<float>& originalContent,
                    const std::vector<float>& newContent)
        : layerId(layerId), rect(rect), 
          originalContent(originalContent),
          newContent(newContent) {}
    
    void init() override {}
    void execute() override;
    void undo() override;

    void setCanvas(Canvas* canvas) { this->canvas = canvas; }

private:
    std::string layerId;
    PixelRect rect;
    std::vector<float> originalContent;
    std::vector<float> newContent;
};