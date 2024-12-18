#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "../core/canvas.hpp"
#include "../brush/brush_system.hpp"

class Toolbar {
public:
    Toolbar(Canvas& canvas, BrushSystem& brushSystem) 
        : canvas(canvas), brushSystem(brushSystem) {
        brushSize = 20.0f;
        brushOpacity = 1.0f;
        brushColor[0] = brushColor[1] = brushColor[2] = 0.0f;
    }

    void render() {
        setupToolbarStyle();
        
        // Setup toolbar window
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 60));
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | 
                                ImGuiWindowFlags_NoResize | 
                                ImGuiWindowFlags_NoCollapse |
                                ImGuiWindowFlags_NoTitleBar;
        
        ImGui::Begin("Toolbar", nullptr, flags);
        
        // Layout parameters
        float spacing = 10.0f;
        float colorPickerWidth = 180.0f;
        float sliderWidth = 120.0f;
        
        // Color picker
        ImGui::SetNextItemWidth(colorPickerWidth);
        bool colorChanged = ImGui::ColorEdit3("##Color", brushColor, 
                           ImGuiColorEditFlags_NoLabel);
        
        // Size slider
        ImGui::SameLine(0, spacing);
        ImGui::SetNextItemWidth(sliderWidth);
        bool sizeChanged = ImGui::SliderFloat("Size", &brushSize, 1.0f, 50.0f, "%.1f");
        
        // Opacity slider
        ImGui::SameLine(0, spacing);
        ImGui::SetNextItemWidth(sliderWidth);
        bool opacityChanged = ImGui::SliderFloat("Opacity", &brushOpacity, 0.0f, 1.0f, "%.2f");
        
        // Separator
        ImGui::SameLine(0, spacing * 2);
        float separatorHeight = ImGui::GetWindowHeight() - 16.0f;
        float cursorX = ImGui::GetCursorPosX();
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(cursorX, 8.0f), 
            ImVec2(cursorX, separatorHeight),
            ImGui::GetColorU32(ImGuiCol_Separator)
        );
        ImGui::SetCursorPosX(cursorX + 1.0f);
        
        // Add layer button
        ImGui::SameLine(0, spacing * 2);
        if (ImGui::Button("Add Layer")) {
            canvas.addLayer();
        }
        
        // Update brush settings if they changed
        if (colorChanged || sizeChanged || opacityChanged) {
            Color color(brushColor[0], brushColor[1], brushColor[2], brushOpacity);
            brushSystem.updateBrushSettings(brushSize, color);
        }
        
        ImGui::End();
        
        // Restore default style
        restoreDefaultStyle();
    }

private:
    Canvas& canvas;
    BrushSystem& brushSystem;
    float brushColor[3];
    float brushSize;
    float brushOpacity;
    
    void setupToolbarStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        // Save default style
        savedStyle = style;
        
        // Set style parameters
        style.WindowPadding = ImVec2(10, 10);
        style.ItemSpacing = ImVec2(6, 6);
        style.FramePadding = ImVec2(6, 3);
        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.WindowRounding = 0.0f;
        style.FrameRounding = 4.0f;
        
        // Set colors
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    }
    
    void restoreDefaultStyle() {
        ImGui::GetStyle() = savedStyle;
    }
    
    ImGuiStyle savedStyle;
};