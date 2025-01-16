#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>
#include "../core/canvas.hpp"
#include "../brush/brush_system.hpp"
#include "ui/ui_manager.hpp"

class GamepadSystem {
public:
    GamepadSystem(Canvas& canvas, BrushSystem& brushSystem, UIManager& uiManager);
    ~GamepadSystem() = default;

    void init();
    void update();

    // Gamepad state getters
    bool isGamepadConnected() const { return gamepadPresent; }
    float getCursorX() const { return cursorX; }
    float getCursorY() const { return cursorY; }
    bool isDrawing() const { return drawing; }

private:
    void handleButtons();
    void handleTriggers();
    void handleSticks();

    Canvas& canvas;
    BrushSystem& brushSystem;
    UIManager& uiManager;
    bool gamepadPresent;
    
    // Cursor state
    float cursorX;
    float cursorY;
    float cursorSpeed;
    bool drawing;

    // Configuration
    static constexpr float CURSOR_BASE_SPEED = 400.0f; // Pixels per second
    static constexpr float CURSOR_ACCELERATION = 1.0f;
    static constexpr float DEAD_ZONE = 0.15f;
    static constexpr int PRIMARY_GAMEPAD = GLFW_JOYSTICK_1;

    struct ButtonStates {
        bool wasYPressed = false;
        bool wasXPressed = false;
        bool wasDpadUpPressed = false;
        bool wasDpadRightPressed = false;
        bool wasDpadDownPressed = false;
    } lastButtonStates;
};