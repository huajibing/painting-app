#pragma once
#include "../commands/command_system.hpp"
#include <any>
#include <string>
#include <memory>

// Command for adding a new layer
class AddLayerCommand : public Command {
public:
    AddLayerCommand(const std::string& name,
                    const std::string& id) 
            : layerName(name),
              layerId(id) {}
    
    void init() override {};
    void execute() override;
    void undo() override;
    
private:
    std::string layerName;
    std::string layerId;
};

// Command for removing a layer
class RemoveLayerCommand : public Command {
public:
    RemoveLayerCommand(const std::string& id,
                       size_t index) 
                : layerId(id),
                  layerIndex(index) {}
    
    void init() override;
    void execute() override;
    void undo() override;
    
private:
    size_t layerIndex;
    std::string layerName;
    std::string layerId;
    bool isVisible;
    float opacity;
    BlendMode blendMode;
    std::vector<float> layerContent;
};

// Command for duplicating a layer
class DuplicateLayerCommand : public Command {
public:
    DuplicateLayerCommand(const std::string& sourceId, const std::string& duplicatedId) 
        : sourceLayerId(sourceId), duplicatedLayerId(duplicatedId) {}
    
    void init() override {};
    void execute() override;
    void undo() override;

private:
    std::string sourceLayerId;
    std::string duplicatedLayerId;
};

// Command for moving a layer
class MoveLayerCommand : public Command {
public:
    MoveLayerCommand(size_t fromIndex, size_t toIndex) 
        : sourceIndex(fromIndex), targetIndex(toIndex) {}
    
    void init() override {};
    void execute() override;
    void undo() override;
    
private:
    size_t sourceIndex;
    size_t targetIndex;
};

// Command for modifying layer properties
class ModifyLayerCommand : public Command {
public:
    enum class PropertyType {
        Visibility,
        Opacity,
        BlendMode,
        Name
    };
    
    ModifyLayerCommand(const std::string& id, PropertyType type, const std::any& newValue)
        : layerId(id), propertyType(type), newValue(newValue) {}
    
    void init() override;
    void execute() override;
    void undo() override;
    
private:
    std::string layerId;
    PropertyType propertyType;
    std::any newValue;
    std::any oldValue;
    
    void applyValue(const std::any& value);
};

// Command for clearing a layer
class ClearLayerCommand : public Command {
public:
    ClearLayerCommand(const std::string& id)
        : layerId(id) {}
    
    void init() override;
    void execute() override;
    void undo() override;

private:
    std::string layerId;
    std::vector<float> layerContent;
};