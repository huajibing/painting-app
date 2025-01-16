#pragma once
#include <memory>
#include <nfd.hpp>
#include "canvas.hpp"
#include "ui/ui_manager.hpp"
#include "brush/brush_system.hpp"
#include "input/gamepad_system.hpp"

class Application {
public:
    Application();
    ~Application();
    
    void run();
    bool init();

private:
    void handleEvents();
    
    std::unique_ptr<Canvas> canvas;
    std::unique_ptr<UIManager> uiManager;
    std::unique_ptr<BrushSystem> brushSystem;
    std::unique_ptr<FileSystem> fileSystem; 
    std::unique_ptr<GamepadSystem> gamepadSystem;
    bool shouldClose;
};