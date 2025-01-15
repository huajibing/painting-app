#pragma once
#include <memory>
#include "canvas.hpp"
#include "../ui/ui_manager.hpp"
#include "../brush/brush_system.hpp"

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
    bool shouldClose;
};