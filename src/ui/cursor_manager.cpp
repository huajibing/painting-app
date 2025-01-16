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
    if (ImGui::GetTopMostPopupModal() != nullptr) {
        // Handle modal popup cursor
        GLFWcursor* cursor = nullptr;
        switch (ImGui::GetMouseCursor()) {
            case ImGuiMouseCursor_Hand:
                cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
                break;
            case ImGuiMouseCursor_TextInput:
                cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
                break;
            default:
                cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
                break;
        }
        glfwSetCursor(window, cursor);
        return;
    }
    if (!isOverCanvas) {
        // Handle ImGui hover states
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            // Let ImGui control the cursor
            GLFWcursor* cursor = nullptr;
            switch (ImGui::GetMouseCursor()) {
                case ImGuiMouseCursor_Hand:
                    cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
                    break;
                case ImGuiMouseCursor_ResizeNS:
                    cursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
                    break;
                case ImGuiMouseCursor_ResizeEW:
                    cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
                    break;
                case ImGuiMouseCursor_ResizeAll:
                    cursor = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
                    break;
                case ImGuiMouseCursor_TextInput:
                    cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
                    break;
                default:
                    cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
                    break;
            }
            glfwSetCursor(window, cursor);
            return;
        }
        // Default cursor for non-canvas areas
        glfwSetCursor(window, toolCursors[Tool::Pointer]);
        return;
    }

    // Handle canvas area cursors
    auto it = toolCursors.find(currentTool);
    if (it != toolCursors.end() && it->second) {
        glfwSetCursor(window, it->second);
    } else {
        glfwSetCursor(window, toolCursors[Tool::Pointer]);
    }
}