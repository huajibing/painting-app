#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../brush/brush_system.hpp"
#include "../core/canvas.hpp"
#include "layer_panel.hpp"
#include "toolbar.hpp"

class UIManager {
public:
    UIManager();
    ~UIManager();
    
    bool init();
    void render();
    bool shouldClose() const;
    GLFWwindow* getWindow() const { return window; }
    void setBrushSystem(BrushSystem* bs) { brushSystem = bs; }
    void setCanvas(Canvas* c) { canvas = c; }
    void initToolbar() {
        if (canvas) {
            layerPanel = std::make_unique<LayerPanel>(*canvas);
        }
        if (canvas && brushSystem) {
            toolbar = std::make_unique<Toolbar>(*canvas, *brushSystem);
        }
    }
    
private:
    GLFWwindow* window;
    BrushSystem* brushSystem;
    Canvas* canvas;
    std::unique_ptr<LayerPanel> layerPanel;
    std::unique_ptr<Toolbar> toolbar;
    void setupStyle();

    float brushColor[3];
    float brushSize;
    float brushOpacity;
};