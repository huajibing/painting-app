#include "ui_manager.hpp"
#include <imgui.h>
#include <imgui_internal.h>
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
    
    window = glfwCreateWindow(1680, 1000, "Painting App", nullptr, nullptr);
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

    cursorManager = std::make_unique<CursorManager>(window);
    
    return true;
}

void UIManager::render() {
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // Get window size
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    
    // Constants
    const float leftToolbarWidth = 100.0f;
    const float rightPanelWidth = 380.0f;
    const float menuBarHeight = 70.0f;
    
    // Main menu bar
    renderMainMenuBar(menuBarHeight);
    
    // Left toolbar
    renderLeftToolbar(leftToolbarWidth, menuBarHeight, height);
    
    // Right panel
    renderRightPanel(width - rightPanelWidth, menuBarHeight, rightPanelWidth, height - menuBarHeight);
    
    // Canvas area
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(40, 40));
    renderCanvasArea(leftToolbarWidth, menuBarHeight, 
                    width - leftToolbarWidth - rightPanelWidth, 
                    height - menuBarHeight);
    ImGui::PopStyleVar();

    // Popup
    if (showHelpPopup) {
        renderHelpPopup();
    }
    
    // Render ImGui
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
    
    // Load custom font
    io.Fonts->Clear();

    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig icons_config; 
    icons_config.MergeMode = true; 
    icons_config.PixelSnapH = true;
    icons_config.GlyphOffset.y = 1.0f;

    boldFont = io.Fonts->AddFontFromFileTTF("assets/fonts/OpenSans-Bold.ttf", 28.0f);
    io.Fonts->AddFontFromFileTTF("assets/fonts/fa-solid-900.ttf", 24.0f, &icons_config, icons_ranges);

    regularFont = io.Fonts->AddFontFromFileTTF("assets/fonts/OpenSans-Regular.ttf", 28.0f);
    io.Fonts->AddFontFromFileTTF("assets/fonts/fa-solid-900.ttf", 24.0f, &icons_config, icons_ranges);
    
    largeRegularFont = io.Fonts->AddFontFromFileTTF("assets/fonts/OpenSans-Regular.ttf", 33.0f);
    
    largeBoldFont = io.Fonts->AddFontFromFileTTF("assets/fonts/OpenSans-Bold.ttf", 33.0f);
    io.Fonts->AddFontFromFileTTF("assets/fonts/fa-solid-900.ttf", 24.0f, &icons_config, icons_ranges);
    
    icons_config.MergeMode = false;
    largeIconFont = io.Fonts->AddFontFromFileTTF("assets/fonts/fa-solid-900.ttf", 36.0f, &icons_config, icons_ranges);

    io.Fonts->Build();
    
    // Colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.129f, 0.160f, 0.216f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.129f, 0.160f, 0.216f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.129f, 0.160f, 0.216f, 1.00f);
    
    // Headers
    colors[ImGuiCol_Header] = ImVec4(0.129f, 0.160f, 0.216f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.274f, 0.341f, 0.455f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.263f, 0.376f, 0.918f, 1.00f);
    
    // Buttons
    colors[ImGuiCol_Button] = ImVec4(0.129f, 0.160f, 0.216f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.274f, 0.341f, 0.455f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.263f, 0.376f, 0.918f, 1.00f);
    
    // Frame colors (for sliders, input fields etc)
    colors[ImGuiCol_FrameBg] = ImVec4(0.223f, 0.255f, 0.318f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.274f, 0.341f, 0.455f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.263f, 0.376f, 0.918f, 1.00f);
    
    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.129f, 0.160f, 0.216f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.274f, 0.341f, 0.455f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.263f, 0.376f, 0.918f, 1.00f);
    
    // Title
    colors[ImGuiCol_TitleBg] = ImVec4(0.129f, 0.137f, 0.160f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.129f, 0.137f, 0.160f, 1.00f);
    
    // Text
    colors[ImGuiCol_Text] = ImVec4(0.937f, 0.937f, 0.937f, 1.00f);
    
    // Slider
    colors[ImGuiCol_SliderGrab] = ImVec4(0.267f, 0.616f, 0.967f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.367f, 0.716f, 1.000f, 1.00f);
    
    // Separator
    colors[ImGuiCol_Separator] = ImVec4(0.274f, 0.341f, 0.455f, 1.00f);

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.223f, 0.255f, 0.318f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.274f, 0.341f, 0.455f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.374f, 0.441f, 0.555f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.32f, 0.67f, 0.98f, 1.0f);
    
    // Style
    style.WindowPadding = ImVec2(15, 15);
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.WindowRounding = 0.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 4.0f;
    
    // Borders
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
    
    // Scrollbar settings
    style.ScrollbarSize = 16.0f;
    style.ScrollbarRounding = 12.0f;
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

void UIManager::renderMainMenuBar(float height) {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, height));
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 0));
    
    if (ImGui::Begin("##MainMenuBar", nullptr, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
        ImGuiWindowFlags_NoScrollWithMouse)) {

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
        
        // Buttons
        ImVec2 buttonSize_1(140.0f, 50.0f);
        if (ImGui::Button(ICON_FA_PLUS "   New", buttonSize_1)) {
            handleNewFile();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        ImGui::SetItemTooltip("New (Ctrl+N)");

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FOLDER_OPEN "   Open", buttonSize_1)) {
            showOpenDialog();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        ImGui::SetItemTooltip("Open (Ctrl+O)");
        
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_SAVE "   Save", buttonSize_1)) {
            showSaveDialog();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        ImGui::SetItemTooltip("Save (Ctrl+S)");

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FILE_EXPORT "   Export", buttonSize_1)) {
            showExportDialog("png");
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        ImGui::SetItemTooltip("Export (Ctrl+E)");
        
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        
        // Undo/Redo buttons
        ImVec2 buttonSize_2(50.0f, 50.0f);
        ImGui::BeginDisabled(!canvas || !canvas->canUndo());
        if (ImGui::Button(ICON_FA_UNDO "##Undo", buttonSize_2)) {
            canvas->undo();
        }
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        ImGui::SetItemTooltip("Undo (Ctrl+Z)");
        
        ImGui::SameLine();
        ImGui::BeginDisabled(!canvas || !canvas->canRedo());
        if (ImGui::Button(ICON_FA_REDO "##Redo", buttonSize_2)) {
            canvas->redo();
        }
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        ImGui::SetItemTooltip("Redo (Ctrl+Y)");

        ImGui::PopStyleVar();
    }

    float windowWidth = ImGui::GetIO().DisplaySize.x;
    
    // Help button position
    ImVec2 buttonPos(windowWidth - 50, 10);
    ImGui::SetCursorPos(ImVec2(buttonPos.x - ImGui::GetStyle().WindowPadding.x, buttonPos.y));
    
    ImVec2 buttonSize(50.0f, 50.0f);
    if (ImGui::Button(ICON_FA_QUESTION_CIRCLE "##Help", buttonSize)) {
        showHelpPopup = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        ImGui::SetTooltip("Show Help");
    }

    // Gamepad text
    ImGui::SetCursorPos(ImVec2(windowWidth - 800, 20));
    if (hasGamepadCursor) {
        ImGui::Text(ICON_FA_GAMEPAD "  Gamepad connected: %s", gamepadName.c_str());
    }

    // Coordinate overlay
    renderCoordinateOverlay();
    ImGui::End();
    
    ImGui::PopStyleVar(2);
}

void UIManager::renderLeftToolbar(float width, float yOffset, float height) {
    ImGui::SetNextWindowPos(ImVec2(0, yOffset));
    ImGui::SetNextWindowSize(ImVec2(width, height - yOffset));
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 20));
    
    if (ImGui::Begin("##LeftToolbar", nullptr, 
        ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoScrollbar)) {
        
        // Tool buttons
        const float buttonSize = 70.0f;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 8));
        ImGui::PushFont(largeIconFont);
        
        // Brush tool
        if (canvas->getTool() == Tool::Brush) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.263f, 0.376f, 0.918f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.263f, 0.376f, 0.918f, 1.00f));
        }
        if (ImGui::Button(ICON_FA_PENCIL, ImVec2(buttonSize, buttonSize))) {
            canvas->setTool(Tool::Brush);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.129f, 0.160f, 0.216f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.274f, 0.341f, 0.455f, 1.00f));
            
        // Eraser tool
        if (canvas->getTool() == Tool::Eraser) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.263f, 0.376f, 0.918f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.263f, 0.376f, 0.918f, 1.00f));
        }
        if (ImGui::Button(ICON_FA_ERASER, ImVec2(buttonSize, buttonSize))) {
            canvas->setTool(Tool::Eraser);
            brushSystem->setBrushType(BrushType::BaseCircle);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.129f, 0.160f, 0.216f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.274f, 0.341f, 0.455f, 1.00f));

        // Pointer tool
        if (canvas->getTool() == Tool::Pointer) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.263f, 0.376f, 0.918f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.263f, 0.376f, 0.918f, 1.00f));
        }
        if (ImGui::Button(ICON_FA_ARROW_POINTER, ImVec2(buttonSize, buttonSize))) {
            canvas->setTool(Tool::Pointer);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.129f, 0.160f, 0.216f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.274f, 0.341f, 0.455f, 1.00f));
            
        // Selection tool
        if (canvas->getTool() == Tool::Selection) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.263f, 0.376f, 0.918f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.263f, 0.376f, 0.918f, 1.00f));
        }
        if (ImGui::Button(ICON_FA_CROP, ImVec2(buttonSize, buttonSize))) {
            canvas->setTool(Tool::Selection);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }

        ImGui::PopStyleColor(8);            
        ImGui::PopFont();
        ImGui::PopStyleVar();
    }
    ImGui::End();
    
    ImGui::PopStyleVar(2);
}

void UIManager::renderRightPanel(float x, float y, float width, float height) {
    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::SetNextWindowSize(ImVec2(width, height));

    ImGui::PushFont(regularFont);
    
    if (ImGui::Begin("##RightPanel", nullptr, 
        ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove)) {
        
        // Brush Type Section
        std::string brushTypeLabel = getBrushNameByType(brushSystem->getBrushType());
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 8));
        ImGui::PushFont(largeBoldFont);
        ImGui::TextUnformatted(ICON_FA_BRUSH "  Brush Type");
        ImGui::PopFont();
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##BrushType", brushTypeLabel.c_str(), ImGuiComboFlags_NoArrowButton)) {
            for (auto& brushType : brushTypes) {
                if (ImGui::Selectable(getBrushNameByType(brushType).c_str(), brushSystem->getBrushType() == brushType)) {
                    brushSystem->setBrushType(brushType);
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        ImGui::PopStyleVar();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Brush Settings Section
        ImGui::PushFont(largeBoldFont);
        ImGui::TextUnformatted(ICON_FA_GEAR "  Brush Settings");
        ImGui::PopFont();
        ImGui::Spacing();
        // Color picker with custom layout
        ImGui::TextUnformatted("Color");
        ImGui::SameLine();
        bool colorChanged = ImGui::ColorEdit3("##Color", brushColor, 
            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.274f, 0.341f, 0.455f, 1.00f));
        
        // Size slider
        ImGui::TextUnformatted("Size");
        ImGui::SetNextItemWidth(-1);
        bool sizeChanged = ImGui::SliderFloat("##Size", &brushSize, 1.0f, 100.0f, "%.0f");
        
        // Opacity slider
        ImGui::TextUnformatted("Opacity");
        ImGui::SetNextItemWidth(-1);
        bool opacityChanged = ImGui::SliderFloat("##Opacity", &brushOpacity, 0.0f, 1.0f, "%.2f");
        
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Update brush settings if any value changed
        if (colorChanged || sizeChanged || opacityChanged || hasGamepadCursor) {
            if (brushSystem) {
                Color color(brushColor[0], brushColor[1], brushColor[2], brushOpacity);
                brushSystem->updateBrushSettings(brushSize, color);
            }
        }
        
        // Layers Section
        ImGui::PushFont(largeBoldFont);
        ImGui::TextUnformatted(ICON_FA_LAYER_GROUP "  Layers");
        ImGui::PopFont();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.223f, 0.255f, 0.318f, 1.00f));
        if (ImGui::Button(ICON_FA_PLUS "  Add Layer", ImVec2(-1, 50))) {
            if (canvas) canvas->addLayer();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        ImGui::PopStyleColor();
        
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::BeginChild("LayersList", ImVec2(-1, -1), true);
        renderLayersList();
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }
    ImGui::End();
    ImGui::PopFont();
}

void UIManager::renderLayersList() {
    if (!canvas) return;
    
    static int draggedLayer = -1;
    static bool isRenamingLayer = false;
    static char renameBuffer[256] = "";
    static int renamingLayerIndex = -1;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

    // Get layers
    for (int i = canvas->getLayerCount() - 1; i >= 0; i--) {
        Layer* layer = canvas->getLayer(i);
        if (!layer) continue;

        bool isSelected = (i == canvas->getActiveLayerIndex());
        ImGui::PushID(i);

        // Background color
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.274f, 0.341f, 0.455f, 1.00f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.223f, 0.255f, 0.318f, 1.00f));
        }

        ImGui::PushStyleColor(ImGuiCol_Button, 
            isSelected ? ImVec4(0.274f, 0.341f, 0.455f, 1.00f) : ImVec4(0.223f, 0.255f, 0.318f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 
            isSelected ? ImVec4(0.274f, 0.341f, 0.455f, 1.00f) : ImVec4(0.223f, 0.255f, 0.318f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, 
            isSelected ? ImVec4(0.274f, 0.341f, 0.455f, 1.00f) : ImVec4(0.223f, 0.255f, 0.318f, 1.00f));

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
        ImGui::BeginChild("LayerRow", ImVec2(ImGui::GetContentRegionAvail().x, 40), false);

        // Drag handle
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_GRIP_LINES "##DragHandle")) {
            canvas->setActiveLayer(i);
        }
        // Set drag source
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("LAYER_ITEM", &i, sizeof(int));
            ImGui::TextUnformatted(layer->getName().c_str());
            ImGui::EndDragDropSource();
        }
        // Set drag target
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LAYER_ITEM")) {
                int srcIndex = *(const int*)payload->Data;
                canvas->moveLayer(srcIndex, i);
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
            // If not renaming any layer, set active layer
            if (!ImGui::IsAnyItemHovered()) {
                canvas->setActiveLayer(i);
            }
        }

        ImGui::SameLine(40);

        // Visible toggle
        bool visible = layer->getVisibility();
        if (visible) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.f));
        }
        if (ImGui::Button(ICON_FA_EYE "##Visible")) {
            layer->setVisibility(!visible);
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();

        // Rename layer
        if (renamingLayerIndex == i) {
            ImGui::SetNextItemWidth(150);
            ImGuiInputTextFlags input_flags = 
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
            if (ImGui::InputText("##RenameLayer", renameBuffer, sizeof(renameBuffer), input_flags)) {
                if (strlen(renameBuffer) > 0) {
                    layer->setName(renameBuffer, true);
                }
                renamingLayerIndex = -1;
            }
            if (ImGui::IsItemDeactivated()) {
                if (strlen(renameBuffer) > 0) {
                    layer->setName(renameBuffer, true);
                }
                renamingLayerIndex = -1;
            }
        } else {
            if (ImGui::Button(layer->getName().c_str(), ImVec2(150, 0))) {
                // Do nothing
            }
            // Rename layer on double click
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                renamingLayerIndex = i;
                strncpy(renameBuffer, layer->getName().c_str(), sizeof(renameBuffer));
            }
        }

        float offset = ImGui::GetContentRegionAvail().x - 96;
        ImGui::SameLine(offset);

        // Opacity slider
        float opacity = layer->getOpacity();
        int opacityInt = static_cast<int>(opacity * 100);
        ImGui::SetNextItemWidth(50);
        if (ImGui::DragInt("##Opacity", &opacityInt, 1, 0, 100)) {
            layer->setOpacity(opacityInt / 100.f);
        }

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_TRASH "##Delete")) {
            canvas->removeLayer(i);
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);

        ImGui::PopID();
    }

    ImGui::PopStyleVar(2);
}

void UIManager::renderCanvasArea(float x, float y, float width, float height) {
    bool modalIsOpen = (ImGui::GetTopMostPopupModal() != nullptr);

    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::SetNextWindowSize(ImVec2(width, height));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.22f, 0.25f, 0.32f, 1.0f));
    ImGui::Begin("Canvas", nullptr, 
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar | 
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoMouseInputs);
    
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

    ImVec2 mousePos = ImGui::GetMousePos();
    bool isOverCanvas = (mousePos.x >= finalPos.x && mousePos.x < finalPos.x + displaySize.x &&
                        mousePos.y >= finalPos.y && mousePos.y < finalPos.y + displaySize.y);
    
    if (cursorManager && canvas && !modalIsOpen) {
        cursorManager->updateCursor(canvas->getTool(), isOverCanvas);
    }

    if (hasGamepadCursor) {
        renderGamepadCursor();
    }

    ImGui::End();
    ImGui::PopStyleColor();
}

void UIManager::renderHelpPopup() {
    ImGui::SetNextWindowSize(ImVec2(1060, 600));
    
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    
    const ImVec4 titleColor(0.32f, 0.67f, 0.98f, 1.0f);
    const ImVec4 subtitleColor(0.9f, 0.9f, 0.9f, 1.0f);
    const ImVec4 textColor(0.8f, 0.8f, 0.8f, 1.0f);
    
    if (showHelpPopup) {
        ImGui::OpenPopup("Quick Help Guide");
    }
    
    // Style
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30, 30));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 24.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 15));
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | 
                            ImGuiWindowFlags_NoMove | 
                            ImGuiWindowFlags_NoTitleBar;
                            
    if (ImGui::BeginPopupModal("Quick Help Guide", &showHelpPopup, flags)) {
        // Title and close button
        ImGui::BeginGroup();
        ImGui::PushFont(largeBoldFont);
        ImGui::TextColored(titleColor, ICON_FA_BOOK "  Help Guide");
        ImGui::PopFont();
        
        // Close button
        ImGui::SameLine(ImGui::GetWindowWidth() - 100);
        if (ImGui::Button(ICON_FA_XMARK "##close", ImVec2(40, 40))) {
            showHelpPopup = false;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        ImGui::EndGroup();
        
        ImGui::Spacing();
        ImGui::Spacing();

        // Calculate layout positions
        float windowWidth = ImGui::GetWindowWidth();
        float columnWidth = (windowWidth - 75) * 0.5f; // 75 for padding
        float startX = ImGui::GetCursorPosX();
        float startY = ImGui::GetCursorPosY();
        float rightColumnX = startX + columnWidth + 15; // 15 for spacing between columns

        // Left column title
        ImGui::PushFont(boldFont);
        ImGui::TextColored(subtitleColor, ICON_FA_KEYBOARD "  Keyboard Shortcuts");
        ImGui::PopFont();
        ImGui::Separator();
        ImGui::Spacing();

        // Right column title
        float currentY = ImGui::GetCursorPosY();
        ImGui::SetCursorPos(ImVec2(rightColumnX, startY));
        ImGui::PushFont(boldFont);
        ImGui::TextColored(subtitleColor, ICON_FA_TOOLBOX "  Tools");
        ImGui::PopFont();
        ImGui::Spacing();

        ImGui::SetCursorPos(ImVec2(startX, currentY + ImGui::GetStyle().ItemSpacing.y * 2));
        
        // File Operations
        ImGui::BeginGroup();
        ImGui::PushStyleColor(ImGuiCol_Text, textColor);
        ImGui::TextColored(titleColor, "File Operations");
        ImGui::Spacing();
        ImGui::Indent(20);
        ImGui::BulletText("Ctrl + N: New File (Clear All)");
        ImGui::BulletText("Ctrl + O: Open File");
        ImGui::BulletText("Ctrl (+ Shift) + S: Save");
        ImGui::BulletText("Ctrl + E: Export");
        ImGui::Unindent(20);
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::TextColored(titleColor, "Edit Operations");
        ImGui::Spacing();
        ImGui::Indent(20);
        ImGui::BulletText("Ctrl + Z: Undo");
        ImGui::BulletText("Ctrl + Y / Ctrl + Shift + Z: Redo");
        ImGui::BulletText("Ctrl + C: Copy Selection");
        ImGui::BulletText("Ctrl + X: Cut Selection");
        ImGui::BulletText("Ctrl + V: Paste Selection");
        ImGui::BulletText("Delete: Delete Selection");
        ImGui::Unindent(20);
        ImGui::PopStyleColor();
        ImGui::EndGroup();

        float leftHeight = ImGui::GetItemRectSize().y;

        ImGui::SetCursorPos(ImVec2(rightColumnX, currentY + ImGui::GetStyle().ItemSpacing.y * 2));
        
        // Right column content
        ImGui::BeginGroup();
        ImGui::PushStyleColor(ImGuiCol_Text, textColor);
        ImGui::SetNextItemWidth(columnWidth);

        ImGui::TextColored(titleColor, "Drawing Tools");
        ImGui::Spacing();
        ImGui::Indent(20);
        ImGui::BulletText(ICON_FA_PENCIL "  Brush: Free-form drawing");
        ImGui::BulletText(ICON_FA_ERASER "  Eraser: Erase parts of image");
        ImGui::Unindent(20);
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::TextColored(titleColor, "Selection Tools");
        ImGui::Spacing();
        ImGui::Indent(20);
        ImGui::BulletText(ICON_FA_CROP "  Selector: Select & manipulate");
        ImGui::BulletText(ICON_FA_ARROW_POINTER "  Pointer: Basic cursor tool");
        ImGui::Unindent(20);
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::TextColored(titleColor, "Layer Operations");
        ImGui::Spacing();
        ImGui::Indent(20);
        ImGui::BulletText(ICON_FA_LAYER_GROUP "  Layer panel: Manage layers");
        ImGui::BulletText(ICON_FA_EYE "  Toggle visibility of layers");
        ImGui::Unindent(20);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::TextColored(titleColor, "Gamepad Controls");
        ImGui::Spacing();
        ImGui::Indent(20);
        ImGui::BulletText("Left Stick: Move cursor");
        ImGui::BulletText("Right Bumper (RB/R1): Draw");
        ImGui::BulletText("Right Trigger (RT/R2): Accelerate cursor");
        ImGui::BulletText("Left Trigger (LT/L2): Adjust brush size");
        ImGui::BulletText("D-Pad Up: Select Brush tool");
        ImGui::BulletText("D-Pad Right: Select Eraser tool");
        ImGui::BulletText("D-Pad Down: Select Selection tool");
        ImGui::BulletText("Y Button: Undo");
        ImGui::BulletText("X Button: Redo");
        ImGui::Unindent(20);
        
        ImGui::PopStyleColor(2);
        ImGui::EndGroup();

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(3);
}


void UIManager::renderCoordinateOverlay() {
    if (!canvas) return;

    ImVec2 mousePos = ImGui::GetMousePos();
    float canvasX, canvasY;
    
    if (windowToCanvas(mousePos.x, mousePos.y, canvasX, canvasY)) {
        float windowWidth = ImGui::GetIO().DisplaySize.x;
        ImVec2 textPos(windowWidth - 250, 20);
        
        char coordText[64];
        snprintf(coordText, sizeof(coordText), "X: %.0f  Y: %.0f", canvasX, canvasY);
        
        ImGui::SetCursorPos(ImVec2(textPos.x - ImGui::GetStyle().WindowPadding.x, textPos.y));
        ImGui::TextUnformatted(coordText);
    }
}

void UIManager::showOpenDialog() {
    NFD::UniquePath outPath;
    nfdfilteritem_t filterItem[3] = {
        { "Project Files", "paint" },
        { "Image Files", "png,jpg,jpeg" },
        { "All Files", "*" }
    };
    
    nfdresult_t result = NFD::OpenDialog(outPath, filterItem, 3);
    if (result == NFD_OKAY) {
        std::string path = outPath.get();
        std::string ext = getFileExtension(path);
        
        try {
            if (ext == "png" || ext == "jpg" || ext == "jpeg") {
                fileSystem->importImage(path);
            } else if (ext == "paint") {
                fileSystem->loadProject(path);
            }
        } catch (const std::exception& e) {
            // TODO: Show error dialog
            std::cerr << "Failed to open file: " << e.what() << std::endl;
        }
    } else if (result == NFD_ERROR) {
        std::cerr << "Error opening file dialog: " << NFD::GetError() << std::endl;
    }
}

void UIManager::showSaveDialog() {
    NFD::UniquePath outPath;
    nfdfilteritem_t filterItem[2] = {
        { "Project Files", "paint" },
        { "All Files", "*" }
    };
    
    nfdresult_t result = NFD::SaveDialog(outPath, filterItem, 2, nullptr, "untitled.paint");
    if (result == NFD_OKAY) {
        std::string path = outPath.get();
        if (addDefaultExtension(path, "paint")) {
            try {
                fileSystem->saveProject(path);
            } catch (const std::exception& e) {
                // TODO: Show error dialog
                std::cerr << "Failed to save file: " << e.what() << std::endl;
            }
        }
    } else if (result == NFD_ERROR) {
        std::cerr << "Error opening save dialog: " << NFD::GetError() << std::endl;
    }
}

void UIManager::handleExportImage(const std::string& format) {
    showExportDialog(format);
}

void UIManager::showExportDialog(const std::string& defaultFormat) {
    NFD::UniquePath outPath;
    nfdfilteritem_t filterItem[2] = {
        { "Image Files", defaultFormat.c_str() },
        { "All Files", "*" }
    };
    
    std::string defaultName = "untitled." + defaultFormat;
    nfdresult_t result = NFD::SaveDialog(outPath, filterItem, 2, nullptr, defaultName.c_str());
    
    if (result == NFD_OKAY) {
        std::string path = outPath.get();
        if (addDefaultExtension(path, defaultFormat)) {
            try {
                fileSystem->exportImage(path);
            } catch (const std::exception& e) {
                // TODO: Show error dialog
                std::cerr << "Failed to export image: " << e.what() << std::endl;
            }
        }
    } else if (result == NFD_ERROR) {
        std::cerr << "Error opening export dialog: " << NFD::GetError() << std::endl;
    }
}

std::string UIManager::getFileExtension(const std::string& path) {
    size_t dotPos = path.find_last_of(".");
    if (dotPos != std::string::npos) {
        return path.substr(dotPos + 1);
    }
    return "";
}

bool UIManager::addDefaultExtension(std::string& path, const std::string& ext) {
    std::string currentExt = getFileExtension(path);
    if (currentExt.empty()) {
        path += "." + ext;
    }
    return true;
}

void UIManager::handleNewFile() {
    if (canvas) {
        canvas->clear();

        brushSize = 20.0f;
        brushOpacity = 1.0f;
        brushColor[0] = brushColor[1] = brushColor[2] = 0.0f;
    }
}

void UIManager::updateGamepadCursor(float x, float y) {
    hasGamepadCursor = true;
    gamepadCursorX = x;
    gamepadCursorY = y;
}

void UIManager::renderGamepadCursor() {
    if (!hasGamepadCursor || !canvas) return;

    // Convert canvas coordinates to screen coordinates
    float canvasX = gamepadCursorX;
    float canvasY = gamepadCursorY;
    
    // Convert canvas coordinates to screen coordinates
    float screenX = canvasDisplayPos.x + (canvasX / canvas->getWidth()) * canvasDisplaySize.x;
    float screenY = canvasDisplayPos.y + (canvasY / canvas->getHeight()) * canvasDisplaySize.y;

    // Get the window draw list
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    
    // Cursor style
    const float cursorSize = 20.0f;
    const ImU32 cursorColor = IM_COL32(53, 81, 210, 255);
    const ImU32 cursorOutlineColor = IM_COL32(255, 255, 255, 255);
    const float outlineThickness = 3.0f;

    // Draw cursor (crosshair style)
    // Outline
    drawList->AddLine(
        ImVec2(screenX - cursorSize, screenY),
        ImVec2(screenX + cursorSize, screenY),
        cursorOutlineColor,
        outlineThickness
    );
    drawList->AddLine(
        ImVec2(screenX, screenY - cursorSize),
        ImVec2(screenX, screenY + cursorSize),
        cursorOutlineColor,
        outlineThickness
    );

    // Inner lines
    drawList->AddLine(
        ImVec2(screenX - cursorSize + 2, screenY),
        ImVec2(screenX + cursorSize - 2, screenY),
        cursorColor,
        2.0f
    );
    drawList->AddLine(
        ImVec2(screenX, screenY - cursorSize + 2),
        ImVec2(screenX, screenY + cursorSize - 2),
        cursorColor,
        2.0f
    );
}