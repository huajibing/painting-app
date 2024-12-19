#include "application.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

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
        return false;
    }

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Initialize canvas with default size
    canvas = std::make_unique<Canvas>(1600, 1200);
    if (!canvas->init()) {
        return false;
    }

    // Set canvas for UI manager
    uiManager->setCanvas(canvas.get());

    // Set window resize callback
    GLFWwindow* window = uiManager->getWindow();
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
        auto app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        glViewport(0, 0, width, height);
        if (app->canvas) {
            app->canvas->resize(width, height);
        }
    });

    // Initialize brush system after canvas
    brushSystem = std::make_unique<BrushSystem>(*canvas);

    // Connect brush system to UI manager
    uiManager->setBrushSystem(brushSystem.get());

    uiManager->initToolbar();

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;

    int width, height;
    glfwGetFramebufferSize(uiManager->getWindow(), &width, &height);
    canvas->resize(width, height);
        
    return true;
}

void Application::run() {
    while (!shouldClose) {
        handleEvents();
        
        // Clear frame with dark gray background
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        // glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render canvas
        canvas->render();
        
        // Render UI
        uiManager->render();
        
        // Process input and check if window should close
        shouldClose = uiManager->shouldClose();
    }
}

void Application::handleEvents() {
    glfwPollEvents();
    
    GLFWwindow* window = uiManager->getWindow();
    
    // Check if mouse is over ImGui windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    static bool wasPressed = false;
    bool isPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    // Mouse button just pressed (start new stroke)
    if (isPressed && !wasPressed) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        float canvasX, canvasY;
        if (canvas->windowToCanvas(xpos, ypos, canvasX, canvasY)) {
            brushSystem->beginStroke();
            brushSystem->draw(canvasX, canvasY);
        }
    }
    // Mouse button is being held
    else if (isPressed) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        float canvasX, canvasY;
        if (canvas->windowToCanvas(xpos, ypos, canvasX, canvasY)) {
            brushSystem->draw(canvasX, canvasY);
        }
    }
    // Mouse button just released (end stroke)
    else if (!isPressed && wasPressed) {
        brushSystem->endStroke();
    }

    wasPressed = isPressed;
}