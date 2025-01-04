#pragma once
#include <imgui.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../brush/brush_system.hpp"
#include "../core/canvas.hpp"

class UIManager {
public:
    UIManager();
    ~UIManager() { cleanup(); }
    
    bool init();
    void render();
    bool shouldClose() const;
    
    void setBrushSystem(BrushSystem* bs);
    void setCanvas(Canvas* c);
    
    GLFWwindow* getWindow() const { return window; }

    // Canvas coordinate conversion
    bool windowToCanvas(double windowX, double windowY, float& canvasX, float& canvasY) const {
        if (!canvas) return false;
        
        // Check if the point is within the canvas area
        if (windowX < canvasDisplayPos.x || windowX >= canvasDisplayPos.x + canvasDisplaySize.x ||
            windowY < canvasDisplayPos.y || windowY >= canvasDisplayPos.y + canvasDisplaySize.y) {
            return false;
        }

        // Convert to canvas coordinates
        float normalizedX = (windowX - canvasDisplayPos.x) / canvasDisplaySize.x;
        float normalizedY = (windowY - canvasDisplayPos.y) / canvasDisplaySize.y;
        
        canvasX = normalizedX * canvas->getWidth();
        canvasY = normalizedY * canvas->getHeight();
        
        return true;
    }

private:
    void cleanup();
    void setupStyle();
    
    GLFWwindow* window;
    BrushSystem* brushSystem;
    Canvas* canvas;
    
    // UI state
    float brushColor[3];
    float brushSize;
    float brushOpacity;
    bool sidebarVisible;
    
    // Canvas display info
    ImVec2 canvasDisplayPos;
    ImVec2 canvasDisplaySize;
    
    // Cache last frame state
    ImVec2 lastWindowSize;
    bool lastFrameValid = false;
};