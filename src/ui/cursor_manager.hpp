#pragma once
#include <GLFW/glfw3.h>
#include "../core/canvas.hpp"
#include <unordered_map>
#include "imgui_internal.h"

class CursorManager {
public:
    CursorManager(GLFWwindow* window);
    ~CursorManager();

    void updateCursor(Tool currentTool, bool isOverCanvas);

private:
    GLFWwindow* window;
    std::unordered_map<Tool, GLFWcursor*> toolCursors;
    
    void createCursors();
    void destroyCursors();
};