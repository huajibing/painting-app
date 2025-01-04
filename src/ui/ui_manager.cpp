#include "ui_manager.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

UIManager::UIManager() 
    : window(nullptr), brushSystem(nullptr),
    canvas(nullptr), brushSize(20.0f), brushOpacity(1.0f),
    sidebarVisible(true) {
    brushColor[0] = 0.0f;
    brushColor[1] = 0.0f;
    brushColor[2] = 0.0f;
}

bool UIManager::init() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(1600, 1000, "Painting App", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return false;
    }
    
    glfwMakeContextCurrent(window);
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    
    // Enable vsync
    glfwSwapInterval(1);
    
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    // Setup ImGui style
    setupStyle();
    
    return true;
}

void UIManager::render() {
    // Get buffer size
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    
    // Clear the background
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Get window size
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Calculate sidebar width
    float sidebarWidth = sidebarVisible ? 300.0f : 0.0f;

    // Main menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {}
            if (ImGui::MenuItem("Open")) {}
            if (ImGui::MenuItem("Save")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(window, true);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {canvas->undo();}
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {canvas->redo();}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Show Sidebar", NULL, &sidebarVisible);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Sidebar
    if (sidebarVisible) {
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
        ImGui::SetNextWindowSize(ImVec2(sidebarWidth, height - ImGui::GetFrameHeight()));
        ImGui::Begin("Tools", nullptr, 
            ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoCollapse);

        // Brush settings section
        if (ImGui::CollapsingHeader("Brush Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Color picker
            ImGui::PushItemWidth(-1);
            bool colorChanged = ImGui::ColorEdit3("#olor", brushColor);
            ImGui::PopItemWidth();
            ImGui::Text("Color");
            
            // Brush size and opacity sliders
            ImGui::PushItemWidth(-1);
            bool sizeChanged = ImGui::SliderFloat("Size", &brushSize, 1.0f, 100.0f, "%.0f px");
            bool opacityChanged = ImGui::SliderFloat("Opacity", &brushOpacity, 0.0f, 1.0f, "%.2f");
            ImGui::PopItemWidth();
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // Update brush settings if any value changed
            if (colorChanged || sizeChanged || opacityChanged) {
                if (brushSystem) {
                    Color color(brushColor[0], brushColor[1], brushColor[2], brushOpacity);
                    brushSystem->updateBrushSettings(brushSize, color);
                }
            }
        }

        // Layers section
        if (ImGui::CollapsingHeader("Layers", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15, 8));
            if (ImGui::Button("Add Layer", ImVec2(-1, 0))) {
                if (canvas) {
                    canvas->addLayer();
                }
            }
            ImGui::PopStyleVar();
    
            ImGui::Spacing();

            // List of layers
            ImGui::BeginChild("Layers", ImVec2(0, 250), true);
            if (canvas) {
                for (int i = canvas->getLayerCount() - 1; i >= 0; i--) {
                    Layer* layer = canvas->getLayer(i);
                    if (!layer) continue;

                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));

                    bool isActive = (i == canvas->getActiveLayerIndex());
                    if (ImGui::Selectable(layer->getName().c_str(), isActive)) {
                        canvas->setActiveLayer(i);
                    }

                    // Layer controls
                    float controlWidth = 120.0f;
                    ImGui::SameLine(ImGui::GetWindowWidth() - controlWidth);
                    
                    // Visibility toggle
                    bool visible = layer->getVisibility();
                    if (ImGui::Checkbox(("##visible" + std::to_string(i)).c_str(), &visible)) {
                        layer->setVisibility(visible);
                    }

                    // Opacity slider
                    ImGui::SameLine();
                    float opacity = layer->getOpacity();
                    ImGui::SetNextItemWidth(60);
                    if (ImGui::SliderFloat(("##opacity" + std::to_string(i)).c_str(), 
                                         &opacity, 0.0f, 1.0f, "%.2f")) {
                        layer->setOpacity(opacity);
                    }
                }
            }
            ImGui::EndChild();
        }

        ImGui::End();
    }

    // Main canvas area
    ImGui::SetNextWindowPos(ImVec2(sidebarWidth, ImGui::GetFrameHeight()));
    ImGui::SetNextWindowSize(ImVec2(width - sidebarWidth, height - ImGui::GetFrameHeight()));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 30.0f));
    ImGui::Begin("Canvas", nullptr, 
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar | 
        ImGuiWindowFlags_NoScrollWithMouse);

    // Display FPS in the corner
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 120, 10));
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    // Get canvas area size and position
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    
    // Calculate canvas display size maintaining aspect ratio
    float canvasAspect = (float)canvas->getWidth() / canvas->getHeight();
    float windowAspect = canvasSize.x / canvasSize.y;
    ImVec2 displaySize;
    ImVec2 displayOffset;
    
    if (windowAspect > canvasAspect) {
        displaySize.y = canvasSize.y;
        displaySize.x = displaySize.y * canvasAspect;
        displayOffset.x = (canvasSize.x - displaySize.x) * 0.5f;
        displayOffset.y = 0;
    } else {
        displaySize.x = canvasSize.x;
        displaySize.y = displaySize.x / canvasAspect;
        displayOffset.x = 0;
        displayOffset.y = (canvasSize.y - displaySize.y) * 0.5f;
    }

    // Center the canvas in the window
    ImVec2 finalPos = ImVec2(
        canvasPos.x + displayOffset.x,
        canvasPos.y + displayOffset.y
    );

    // Draw canvas background
    ImGui::GetWindowDrawList()->AddRectFilled(
        finalPos,
        ImVec2(finalPos.x + displaySize.x, finalPos.y + displaySize.y),
        IM_COL32(255, 255, 255, 255)  // White background
    );

    // Get the texture from canvas
    if (canvas) {
        ImTextureID canvasTexture = (ImTextureID)(intptr_t)canvas->getCompositeTexture();
        ImGui::SetCursorScreenPos(finalPos);
        ImGui::Image(canvasTexture, displaySize, ImVec2(0, 1), ImVec2(1, 0));

        // Make the canvas area interactive
        ImGui::SetCursorScreenPos(finalPos);
        ImGui::InvisibleButton("canvas", displaySize, 
            ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
            
        // Store canvas display info for coordinate conversion
        canvasDisplayPos = finalPos;
        canvasDisplaySize = displaySize;
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    if (window) {
        glfwDestroyWindow(window);
    }
}

void UIManager::setupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("assets/fonts/OpenSans-Bold.ttf", 24.0f);
    io.Fonts->Build();
    
    // Colors
    // style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    // style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    // style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    // style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    // style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    // style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    // style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    
    // Rounding
    style.WindowRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    
    // Padding
    style.WindowPadding = ImVec2(15.0f, 15.0f);
    style.FramePadding = ImVec2(10.0f, 10.0f);
    style.ItemSpacing = ImVec2(12.0f, 12.0f);
    style.ItemInnerSpacing = ImVec2(10.0f, 10.0f);
    
    // Sizing
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 22.0f;
    style.GrabMinSize = 22.0f;
    
    // Window sizing
    style.WindowMinSize = ImVec2(250.0f, 120.0f);
    
    // Borders
    style.FrameBorderSize = 1.0f;
    style.WindowBorderSize = 1.0f;
}

void UIManager::setBrushSystem(BrushSystem* bs) {
    brushSystem = bs;
}

void UIManager::setCanvas(Canvas* c) {
    canvas = c;
}

bool UIManager::shouldClose() const {
    return glfwWindowShouldClose(window);
}