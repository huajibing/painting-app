#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include "../selection/selection_system.hpp"
#include "../utils/color.hpp"
#include "../utils/shader.hpp"
#include "layer.hpp"
#include "../commands/command_system.hpp"

struct CanvasTransform {
    ImVec2 displayPos;
    ImVec2 displaySize;
    int textureWidth;
    int textureHeight;
};

enum class Tool {
    Pointer,
    Brush,
    Eraser,
    Selection
};

class Canvas {
public:
    Canvas(int textureWidth, int textureHeight);
    ~Canvas();
    
    bool init();
    void render();
    void resize(int windowWidth, int windowHeight);
    void clear();

    // Command system methods
    CommandManager* getCommandManager() { return commandManager.get(); }
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;
    
    // Stroking operations
    void setStrokeTexture(unsigned int tex) { currentStrokeTexture = tex; }
    void setStroking(bool stroking) { isStroking = stroking; }
    
    // Layer management
    void addLayer(const std::string& name = "New Layer", bool addToCommandStack = true);
    void removeLayer(size_t index, bool addToCommandStack = true);
    void setActiveLayer(size_t index);
    void moveLayer(size_t sourceIdx, size_t targetIdx, bool addToCommandStack = true);
    void duplicateLayer(size_t index, bool addToCommandStack = true);
    size_t getActiveLayerIndex() const { return activeLayerIndex; }
    size_t getLayerCount() const { return layers.size(); }
    Layer* getLayer(size_t index);
    Layer* getLayerById(const std::string& id);
    size_t getLayerIndexById(const std::string& id);
    std::vector<std::unique_ptr<Layer>>& getLayers() { return layers; }
    
    // Dimension getters
    int getWidth() const { return textureWidth; }
    int getHeight() const { return textureHeight; }
    
    // Canvas rectangle and coordinate conversion
    void getCanvasRect(int& x, int& y, int& w, int& h) const;

    // Composite texture access
    unsigned int getCompositeTexture() const;

    // Tool
    void setTool(Tool tool) { currentTool = tool; };
    Tool getTool() const { return currentTool; }

    // Selection
    SelectionSystem* getSelectionSystem() { return selectionSystem.get(); }
    void handleSelectionInput(float x, float y, bool isPressed, bool wasPressed);
    
private:
    void setupQuad();
    void setupCompositeBuffer();
    
    // Canvas dimensions
    int textureWidth;
    int textureHeight;
    int windowWidth;
    int windowHeight;
    
    // Canvas properties
    Color backgroundColor;
    glm::mat4 projection;
    float canvasScale;
    
    // Rendering objects
    unsigned int VAO, VBO, EBO;
    unsigned int compositeFramebuffer;
    unsigned int compositeTexture;
    
    // Layers
    std::vector<std::unique_ptr<Layer>> layers;
    size_t activeLayerIndex;
    
    // Shaders
    std::unique_ptr<Shader> shader;

    // Command system
    std::unique_ptr<CommandManager> commandManager;

    // Selection
    std::unique_ptr<SelectionSystem> selectionSystem;

    unsigned int currentStrokeTexture;
    bool isStroking;

    Tool currentTool;
};