#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../utils/color.hpp"
#include "../utils/shader.hpp"
#include "layer.hpp"
#include "../commands/command_system.hpp"

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
    bool windowToCanvas(double windowX, double windowY, float& canvasX, float& canvasY) const;
    
private:
    void setupQuad();
    void updateProjection();
    void saveViewport();
    void restoreViewport();
    
    // Canvas dimensions
    int textureWidth;
    int textureHeight;
    int displayWidth;
    int displayHeight;
    int windowWidth;
    int windowHeight;
    GLint savedViewport[4];
    
    // Canvas properties
    Color backgroundColor;
    glm::mat4 projection;
    float canvasScale;
    
    // Rendering objects
    unsigned int VAO, VBO, EBO;
    
    // Layers
    std::vector<std::unique_ptr<Layer>> layers;
    size_t activeLayerIndex;
    
    // Shaders
    std::unique_ptr<Shader> shader;

    // Command system
    std::unique_ptr<CommandManager> commandManager;

    unsigned int currentStrokeTexture;
    bool isStroking;
};