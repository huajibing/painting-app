#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../brush/brush_system.hpp"

class UIManager {
public:
    UIManager();
    ~UIManager();
    
    bool init();
    void render();
    bool shouldClose() const;
    GLFWwindow* getWindow() const { return window; }
    void setBrushSystem(BrushSystem* bs) { brushSystem = bs; }
    
private:
    GLFWwindow* window;
    BrushSystem* brushSystem;
    void setupStyle();

    float brushColor[3];
    float brushSize;
    float brushOpacity;
};