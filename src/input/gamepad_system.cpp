#include "gamepad_system.hpp"
#include <iostream>
#include <cmath>

GamepadSystem::GamepadSystem(Canvas& canvas, BrushSystem& brushSystem, UIManager& uiManager)
    : canvas(canvas), brushSystem(brushSystem), uiManager(uiManager),
      gamepadPresent(false), cursorX(0), cursorY(0),
      cursorSpeed(CURSOR_BASE_SPEED), drawing(false) {
}

void GamepadSystem::init() {
    // Check for gamepad presence
    gamepadPresent = glfwJoystickPresent(PRIMARY_GAMEPAD) == GLFW_TRUE;
    
    if (gamepadPresent) {
        if (glfwJoystickIsGamepad(PRIMARY_GAMEPAD)) {
            std::cout << "Gamepad connected: " << glfwGetGamepadName(PRIMARY_GAMEPAD) << std::endl;
            uiManager.setGamepadName(glfwGetGamepadName(PRIMARY_GAMEPAD));
            
            // Initialize cursor position to canvas center
            cursorX = canvas.getWidth() / 2.0f;
            cursorY = canvas.getHeight() / 2.0f;
        } else {
            std::cout << "Joystick present but not a gamepad" << std::endl;
            gamepadPresent = false;
        }
    }
}

void GamepadSystem::update() {
    if (!gamepadPresent) return;

    GLFWgamepadstate state;
    if (glfwGetGamepadState(PRIMARY_GAMEPAD, &state)) {
        handleSticks();
        handleButtons();
        handleTriggers();
    }
}

void GamepadSystem::handleSticks() {
    GLFWgamepadstate state;
    if (glfwGetGamepadState(PRIMARY_GAMEPAD, &state)) {
        float axisX = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        float axisY = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        
        // Apply dead zone
        if (std::abs(axisX) < DEAD_ZONE) axisX = 0;
        if (std::abs(axisY) < DEAD_ZONE) axisY = 0;
        
        // Update cursor position with delta time
        float deltaTime = 1.0f / 60.0f;
        
        float moveSpeed = cursorSpeed;
        // Accelerate if right trigger is pressed
        moveSpeed *= (2.0f + state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]) * CURSOR_ACCELERATION;
        
        cursorX += axisX * moveSpeed * deltaTime;
        cursorY += axisY * moveSpeed * deltaTime;
        
        // Clamp cursor to canvas bounds
        cursorX = std::clamp(cursorX, 0.0f, static_cast<float>(canvas.getWidth()));
        cursorY = std::clamp(cursorY, 0.0f, static_cast<float>(canvas.getHeight()));

        // Update cursor position in UI
        uiManager.updateGamepadCursor(cursorX, cursorY);
        
        // If drawing, update brush position
        if (drawing) {
            brushSystem.draw(cursorX, cursorY);
        }
    }
}

void GamepadSystem::handleButtons() {
    GLFWgamepadstate state;
    if (glfwGetGamepadState(PRIMARY_GAMEPAD, &state)) {
        // Right trigger for drawing
        bool shouldDraw = state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS;
        
        if (shouldDraw && !drawing) {
            brushSystem.beginStroke();
            drawing = true;
        } else if (!shouldDraw && drawing) {
            brushSystem.endStroke();
            drawing = false;
        }
        
        // Tool selection
        bool dpadUpPressed = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS;
        if (dpadUpPressed && !lastButtonStates.wasDpadUpPressed) {
            canvas.setTool(Tool::Brush);
        }
        lastButtonStates.wasDpadUpPressed = dpadUpPressed;

        bool dpadRightPressed = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS;
        if (dpadRightPressed && !lastButtonStates.wasDpadRightPressed) {
            canvas.setTool(Tool::Eraser);
        }
        lastButtonStates.wasDpadRightPressed = dpadRightPressed;

        bool dpadDownPressed = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS;
        if (dpadDownPressed && !lastButtonStates.wasDpadDownPressed) {
            canvas.setTool(Tool::Selection);
        }
        lastButtonStates.wasDpadDownPressed = dpadDownPressed;
        
        // Undo/Redo
        bool yPressed = state.buttons[GLFW_GAMEPAD_BUTTON_Y] == GLFW_PRESS;
        if (yPressed && !lastButtonStates.wasYPressed) {
            if (canvas.canUndo()) canvas.undo();
        }
        lastButtonStates.wasYPressed = yPressed;

        bool xPressed = state.buttons[GLFW_GAMEPAD_BUTTON_X] == GLFW_PRESS;
        if (xPressed && !lastButtonStates.wasXPressed) {
            if (canvas.canRedo()) canvas.redo();
        }
        lastButtonStates.wasXPressed = xPressed;
    }
}

void GamepadSystem::handleTriggers() {
    GLFWgamepadstate state;
    if (glfwGetGamepadState(PRIMARY_GAMEPAD, &state)) {
        // Left trigger could be used for brush size
        float leftTrigger = (state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] + 1.0f) * 0.5f;
        // Map trigger to brush size range (1-100)
        float newSize = 20.0f + leftTrigger * 80.0f;

        // Blend between current and new size
        float currentSize = uiManager.getBrushSize();
        float finalSize = (currentSize * 0.7f) + (newSize * 0.3f);
        
        uiManager.updateBrushSize(finalSize);
    }
}