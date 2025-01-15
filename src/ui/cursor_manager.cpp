#include "cursor_manager.hpp"

CursorManager::CursorManager(GLFWwindow* window) : window(window) {
    createCursors();
}

CursorManager::~CursorManager() {
    destroyCursors();
}

void CursorManager::createCursors() {
    toolCursors[Tool::Brush] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);

    toolCursors[Tool::Eraser] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);

    toolCursors[Tool::Selection] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);

    toolCursors[Tool::Pointer] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
}

void CursorManager::destroyCursors() {
    for (auto& pair : toolCursors) {
        if (pair.second) {
            glfwDestroyCursor(pair.second);
        }
    }
    toolCursors.clear();
}

void CursorManager::updateCursor(Tool currentTool, bool isOverCanvas) {
    if (!isOverCanvas) {
        glfwSetCursor(window, nullptr);
        return;
    }

    auto it = toolCursors.find(currentTool);
    if (it != toolCursors.end() && it->second) {
        glfwSetCursor(window, it->second);
    } else {
        glfwSetCursor(window, nullptr);
    }
}