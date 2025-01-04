#include "application.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include "../brush/texture_generator.hpp"

void generateBrushTextures() {
    BrushTextureGenerator generator;
    
    int size = 512;
    auto scatterData = generator.generateScatterBrush(size, 600);
    // stbi_write_png("assets/brushes/scatter_brush.png", size, size, 4, 
    //                scatterData.data(), size * 4);

    auto roughData = generator.generateRoughBrush(size, 0.1f, 30.0f, 24);
    // stbi_write_png("assets/brushes/rough_brush.png", size, size, 4, 
    //                roughData.data(), size * 4);
}

Application::Application() : shouldClose(false) {}

Application::~Application() {
    glfwTerminate();
}

bool Application::init() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Create UI Manager first as it will create the window
    uiManager = std::make_unique<UIManager>();
    if (!uiManager->init()) {
        std::cerr << "Failed to initialize UI Manager" << std::endl;
        return false;
    }

    // Get window handle
    GLFWwindow* window = uiManager->getWindow();
    if (!window) {
        std::cerr << "No window created" << std::endl;
        return false;
    }

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // Enable vsync
    glfwSwapInterval(1);

    // Initialize canvas with default size
    canvas = std::make_unique<Canvas>(1600, 1200);
    if (!canvas->init()) {
        std::cerr << "Failed to initialize canvas" << std::endl;
        return false;
    }

    // Initialize brush system
    brushSystem = std::make_unique<BrushSystem>(*canvas);
    brushSystem->setCommandManager(canvas->getCommandManager());

    // Connect systems to UI manager
    uiManager->setCanvas(canvas.get());
    uiManager->setBrushSystem(brushSystem.get());

    // Set window resize callback
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
        auto app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        glViewport(0, 0, width, height);
        if (app->canvas) {
            app->canvas->resize(width, height);
        }
    });

    // Setup keyboard shortcuts
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
        auto app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (action == GLFW_PRESS) {
            // Check for Ctrl+Z (Undo)
            if (key == GLFW_KEY_Z && (mods & GLFW_MOD_CONTROL)) {
                if (mods & GLFW_MOD_SHIFT) {
                    // Ctrl+Shift+Z for Redo
                    if (app->canvas && app->canvas->canRedo()) {
                        app->canvas->redo();
                    }
                } else {
                    // Ctrl+Z for Undo
                    if (app->canvas && app->canvas->canUndo()) {
                        app->canvas->undo();
                    }
                }
            }
            // Alternative Redo shortcut (Ctrl+Y)
            else if (key == GLFW_KEY_Y && (mods & GLFW_MOD_CONTROL)) {
                if (app->canvas && app->canvas->canRedo()) {
                    app->canvas->redo();
                }
            }
        }
    });

    // Initial viewport setup
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    canvas->resize(width, height);

    return true;
}

void Application::run() {
    while (!shouldClose) {
        // Poll and handle events
        glfwPollEvents();
        handleEvents();

        // Get window size
        int display_w, display_h;
        glfwGetFramebufferSize(uiManager->getWindow(), &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        // Clear frame
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        canvas->render();

        // Start rendering UI (which includes canvas)
        uiManager->render();
        
        // Handle events after rendering is done
        handleEvents();

        // Swap buffers
        glfwSwapBuffers(uiManager->getWindow());

        // Check if window should close
        shouldClose = uiManager->shouldClose();
    }
}

void Application::handleEvents() {
    if (!canvas || !brushSystem || !uiManager) return;
    
    GLFWwindow* window = uiManager->getWindow();
    
    // Check if mouse is over ImGui windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse && !io.MouseDown[ImGuiMouseButton_Left]) {
        return;
    }

    static bool wasPressed = false;
    bool isPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    // Mouse button just pressed (start new stroke)
    if (isPressed && !wasPressed) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        float canvasX, canvasY;
        if (uiManager->windowToCanvas(xpos, ypos, canvasX, canvasY) && 
            canvas->getLayer(canvas->getActiveLayerIndex())) {
            brushSystem->beginStroke();
            brushSystem->draw(canvasX, canvasY);
        }
    }
    // Mouse button is being held
    else if (isPressed) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        float canvasX, canvasY;
        if (uiManager->windowToCanvas(xpos, ypos, canvasX, canvasY)) {
            brushSystem->draw(canvasX, canvasY);
        }
    }
    // Mouse button just released (end stroke)
    else if (!isPressed && wasPressed) {
        brushSystem->endStroke();
    }

    wasPressed = isPressed;
}