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

    // Initialize file system
    fileSystem = std::make_unique<FileSystem>(*canvas);

    // Initialize brush system
    brushSystem = std::make_unique<BrushSystem>(*canvas);
    brushSystem->setCommandManager(canvas->getCommandManager());

    // Connect systems to UI manager
    uiManager->setCanvas(canvas.get());
    uiManager->setBrushSystem(brushSystem.get());
    uiManager->setFileSystem(fileSystem.get());

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
        if (!app || !app->uiManager) return;

        if (action == GLFW_PRESS) {
            if (mods & GLFW_MOD_CONTROL) {
                switch (key) {
                    case GLFW_KEY_N:  // Ctrl+N
                        app->uiManager->handleNewFile();
                        break;
                        
                    case GLFW_KEY_O:  // Ctrl+O
                        app->uiManager->showOpenDialog();
                        break;
                        
                    case GLFW_KEY_S:  // Ctrl+S
                        if (mods & GLFW_MOD_SHIFT) {
                            app->uiManager->showSaveDialog();  // Ctrl+Shift+S
                        } else {
                            app->uiManager->handleSaveFile(false);  // Ctrl+S
                        }
                        break;
                        
                    case GLFW_KEY_Z:  // Ctrl+Z
                        if (mods & GLFW_MOD_SHIFT) {
                            if (app->canvas && app->canvas->canRedo()) {
                                app->canvas->redo();  // Ctrl+Shift+Z
                            }
                        } else {
                            if (app->canvas && app->canvas->canUndo()) {
                                app->canvas->undo();  // Ctrl+Z
                            }
                        }
                        break;
                        
                    case GLFW_KEY_Y:  // Ctrl+Y
                        if (app->canvas && app->canvas->canRedo()) {
                            app->canvas->redo();
                        }
                        break;

                    case GLFW_KEY_C:  // Ctrl+C
                        if (app->canvas) {
                            auto selectionSystem = app->canvas->getSelectionSystem();
                            if (selectionSystem) {
                                selectionSystem->copySelection();
                            }
                        }
                        break;

                    case GLFW_KEY_X:  // Ctrl+X
                        if (app->canvas) {
                            auto selectionSystem = app->canvas->getSelectionSystem();
                            if (selectionSystem) {
                                selectionSystem->cutSelection();
                            }
                        }
                        break;

                    case GLFW_KEY_V:  // Ctrl+V
                        if (app->canvas) {
                            auto selectionSystem = app->canvas->getSelectionSystem();
                            if (selectionSystem) {
                                // Get current mouse position for paste location
                                double xpos, ypos;
                                glfwGetCursorPos(w, &xpos, &ypos);
                                float canvasX, canvasY;
                                if (app->uiManager->windowToCanvas(xpos, ypos, canvasX, canvasY)) {
                                    selectionSystem->pasteSelection(canvasX, canvasY);
                                }
                            }
                        }
                        break;
                }
            }
            else if (key == GLFW_KEY_DELETE) {  // Delete key (without Ctrl)
                if (app->canvas) {
                    auto selectionSystem = app->canvas->getSelectionSystem();
                    if (selectionSystem && selectionSystem->hasSelection()) {
                        selectionSystem->deleteSelection();
                    }
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
    
    // Get ImGui IO
    ImGuiIO& io = ImGui::GetIO();
    
    // If ImGui wants to capture mouse input, don't handle canvas interactions
    if (io.WantCaptureMouse) {
        return;
    }

    // Get current mouse button state
    bool isPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    static bool wasPressed = false;
    
    // Get current cursor position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    // Convert window coordinates to canvas coordinates
    float canvasX, canvasY;
    bool isOverCanvas = uiManager->windowToCanvas(xpos, ypos, canvasX, canvasY);

    // Only handle mouse events if we're over the canvas
    if (isOverCanvas) {
        switch (canvas->getTool()) {
            case Tool::Selection:
                canvas->handleSelectionInput(canvasX, canvasY, isPressed, wasPressed);
                break;
                
            case Tool::Brush:
                if (canvas->getLayer(canvas->getActiveLayerIndex())) {
                    if (isPressed && !wasPressed) {
                        brushSystem->beginStroke();
                        brushSystem->draw(canvasX, canvasY);
                    }
                    else if (isPressed) {
                        brushSystem->draw(canvasX, canvasY);
                    }
                    else if (!isPressed && wasPressed) {
                        brushSystem->endStroke();
                    }
                }
                break;

            case Tool::Eraser:
                if (canvas->getLayer(canvas->getActiveLayerIndex())) {
                    if (isPressed && !wasPressed) {
                        brushSystem->beginStroke();
                        brushSystem->draw(canvasX, canvasY);
                    }
                    else if (isPressed) {
                        brushSystem->draw(canvasX, canvasY);
                    }
                    else if (!isPressed && wasPressed) {
                        brushSystem->endStroke();
                    }
                }
                break;

            default:
                break;
        }
    } 
    else if (!isPressed && wasPressed) {
        // End any ongoing operations when mouse released outside canvas
        if (canvas->getTool() == Tool::Brush || canvas->getTool() == Tool::Eraser) {
            brushSystem->endStroke();
        }
        else if (canvas->getTool() == Tool::Selection) {
            SelectionSystem* selectionSystem = canvas->getSelectionSystem();
            if (selectionSystem && selectionSystem->hasSelection()) {
                selectionSystem->endSelection();
            }
        }
    }

    wasPressed = isPressed;
}