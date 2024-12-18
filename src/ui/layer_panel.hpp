#pragma once
#include <string>
#include <memory>
#include "../core/canvas.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class LayerPanel {
public:
    LayerPanel(Canvas& canvas) : canvas(canvas) {}

    void render() {
        if (!ImGui::Begin("Layers")) {
            ImGui::End();
            return;
        }

        // Add new layer button
        if (ImGui::Button("Add Layer")) {
            canvas.addLayer();
        }

        ImGui::Separator();

        // Layer list
        for (int i = canvas.getLayerCount() - 1; i >= 0; i--) {
            Layer* layer = canvas.getLayer(i);
            if (!layer) continue;

            ImGui::PushID(i);
            bool isActive = (i == canvas.getActiveLayerIndex());

            // Layer selection
            if (ImGui::Selectable(layer->getName().c_str(), isActive)) {
                canvas.setActiveLayer(i);
            }

            // Drag and drop source
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                ImGui::SetDragDropPayload("LAYER_ITEM", &i, sizeof(int));
                ImGui::Text("Move %s", layer->getName().c_str());
                ImGui::EndDragDropSource();
            }

            // Drag and drop target
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LAYER_ITEM")) {
                    int sourceIdx = *(const int*)payload->Data;
                    moveLayer(sourceIdx, i);
                }
                ImGui::EndDragDropTarget();
            }

            // Layer controls (right-aligned)
            ImGui::SameLine(ImGui::GetWindowWidth() - 100);
            
            // Visibility toggle
            bool visible = layer->getVisibility();
            if (ImGui::Checkbox("##visible", &visible)) {
                layer->setVisibility(visible);
            }

            // Opacity slider
            ImGui::SameLine();
            float opacity = layer->getOpacity();
            ImGui::PushItemWidth(60);
            if (ImGui::SliderFloat("##opacity", &opacity, 0.0f, 1.0f, "%.2f")) {
                layer->setOpacity(opacity);
            }
            ImGui::PopItemWidth();

            // Layer options popup
            ImGui::SameLine();
            if (ImGui::Button("...")) {
                ImGui::OpenPopup("layer_options");
            }
            
            if (ImGui::BeginPopup("layer_options")) {
                // Rename layer
                static char nameBuf[256];
                strncpy(nameBuf, layer->getName().c_str(), sizeof(nameBuf) - 1);
                if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
                    layer->setName(nameBuf);
                }

                // Delete layer
                if (ImGui::Button("Delete") && canvas.getLayerCount() > 1) {
                    canvas.removeLayer(i);
                    ImGui::CloseCurrentPopup();
                }

                // Blend mode selection
                const char* blendModes[] = {"Normal", "Multiply", "Screen", "Overlay"};
                int currentBlendMode = static_cast<int>(layer->getBlendMode());
                if (ImGui::Combo("Blend Mode", &currentBlendMode, blendModes, 4)) {
                    layer->setBlendMode(static_cast<BlendMode>(currentBlendMode));
                }

                ImGui::EndPopup();
            }

            ImGui::PopID();
        }

        ImGui::End();
    }

private:
    Canvas& canvas;

    void moveLayer(int sourceIdx, int targetIdx) {
        if (sourceIdx == targetIdx) return;
        
        // Get the layers vector from canvas
        std::vector<std::unique_ptr<Layer>>& layers = canvas.getLayers();
        
        // Store the active layer index
        size_t activeIdx = canvas.getActiveLayerIndex();
        
        // Move the layer
        auto temp = std::move(layers[sourceIdx]);
        layers.erase(layers.begin() + sourceIdx);
        layers.insert(layers.begin() + targetIdx, std::move(temp));
        
        // Update active layer index if needed
        if (activeIdx == sourceIdx) {
            canvas.setActiveLayer(targetIdx);
        } else if (activeIdx > sourceIdx && activeIdx <= targetIdx) {
            canvas.setActiveLayer(activeIdx - 1);
        } else if (activeIdx < sourceIdx && activeIdx >= targetIdx) {
            canvas.setActiveLayer(activeIdx + 1);
        }
    }
};