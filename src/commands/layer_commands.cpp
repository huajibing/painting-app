#include "../core/layer.hpp"
#include "layer_commands.hpp"
#include "../core/canvas.hpp"
#include <iostream>

void AddLayerCommand::execute() {
    if (!canvas) return;
    canvas->addLayer(layerName, false);
    Layer* layer = canvas->getLayer(canvas->getLayerCount() - 1);
    if (layer) {
        layer->setId(layerId);
    }
}

void AddLayerCommand::undo() {
    if (!canvas) return;
    canvas->removeLayer(canvas->getLayerIndexById(layerId), false);
}

void RemoveLayerCommand::init() {
    if (!canvas) return;
    // Store layer data for restoration
    Layer* layer = canvas->getLayerById(layerId);
    if (layer) {
        layerName = layer->getName();
        isVisible = layer->getVisibility();
        opacity = layer->getOpacity();
        blendMode = layer->getBlendMode();
        
        // Store layer content
        PixelRect rect(0, 0, canvas->getWidth(), canvas->getHeight());
        layerContent = layer->getPixels(rect);
    }
}

void RemoveLayerCommand::execute() {
    if (!canvas) return;
    canvas->removeLayer(layerIndex, false);
}

void RemoveLayerCommand::undo() {
    if (!canvas) return;
    // Re-add the layer
    canvas->addLayer(layerName);
    Layer* layer = canvas->getLayer(canvas->getLayerCount() - 1);
    if (layer) {
        // Restore layer properties
        layer->setVisibility(isVisible);
        layer->setOpacity(opacity);
        layer->setBlendMode(blendMode);
        
        // Restore layer content
        PixelRect rect(0, 0, canvas->getWidth(), canvas->getHeight());
        layer->setPixels(rect, layerContent);
        
        // Move layer to original position if needed
        if (layerIndex != canvas->getLayerCount() - 1) {
            canvas->getLayers().insert(
                canvas->getLayers().begin() + layerIndex,
                std::move(canvas->getLayers().back())
            );
            canvas->getLayers().pop_back();
        }
    }
}

void DuplicateLayerCommand::execute() {
    if (!canvas) return;
    canvas->duplicateLayer(canvas->getLayerIndexById(sourceLayerId), false);
}

void DuplicateLayerCommand::undo() {
    if (!canvas) return;
    canvas->removeLayer(canvas->getLayerIndexById(duplicatedLayerId), false);
}

void MoveLayerCommand::execute() {
    if (!canvas) return;
    canvas->moveLayer(sourceIndex, targetIndex, false);
}

void MoveLayerCommand::undo() {
    if (!canvas) return;
    // Reverse the move operation
    canvas->moveLayer(targetIndex, sourceIndex, false);
}

void ModifyLayerCommand::init() {
    if (!canvas) return;
    Layer* layer = canvas->getLayerById(layerId);
    if (!layer) return;
    
    // Store old value based on property type
    switch (propertyType) {
        case PropertyType::Visibility:
            oldValue = layer->getVisibility();
            break;
        // case PropertyType::Opacity:
        //     oldValue = layer->getOpacity();
        //     break;
        case PropertyType::BlendMode:
            oldValue = layer->getBlendMode();
            break;
        case PropertyType::Name:
            oldValue = layer->getName();
            break;
    }
}

void ModifyLayerCommand::execute() {
    if (!canvas) return;
    applyValue(newValue);
}

void ModifyLayerCommand::undo() {
    if (!canvas) return;
    applyValue(oldValue);
}

void ModifyLayerCommand::applyValue(const std::any& value) {
    if (!canvas) return;
    Layer* layer = canvas->getLayerById(layerId);
    if (!layer) return;
    
    try {
        switch (propertyType) {
            case PropertyType::Visibility:
                layer->setVisibility(std::any_cast<bool>(value), false);
                break;
            // case PropertyType::Opacity:
            //     layer->setOpacity(std::any_cast<float>(value));
            //     break;
            case PropertyType::BlendMode:
                layer->setBlendMode(std::any_cast<BlendMode>(value), false);
                break;
            case PropertyType::Name:
                layer->setName(std::any_cast<std::string>(value), false);
                break;
        }
    } catch (const std::bad_any_cast& e) {
        std::cerr << "Failed to apply layer property: " << e.what() << std::endl;
    }
}

void ClearLayerCommand::init() {
    if (!canvas) return;
    Layer* layer = canvas->getLayerById(layerId);
    if (layer) {
        // Store layer content
        PixelRect rect(0, 0, canvas->getWidth(), canvas->getHeight());
        layerContent = layer->getPixels(rect);
    }
}

void ClearLayerCommand::execute() {
    if (!canvas) return;
    Layer* layer = canvas->getLayerById(layerId);
    if (layer) {
        layer->clear();
    }
}

void ClearLayerCommand::undo() {
    if (!canvas) return;
    Layer* layer = canvas->getLayerById(layerId);
    if (layer) {
        // Restore layer content
        PixelRect rect(0, 0, canvas->getWidth(), canvas->getHeight());
        layer->setPixels(rect, layerContent);
    }
}