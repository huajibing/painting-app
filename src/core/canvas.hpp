#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../utils/color.hpp"
#include "../utils/shader.hpp"

class Canvas {
public:
    Canvas(int textureWidth, int textureHeight);
    ~Canvas();
    
    bool init();
    void render();
    void resize(int windowWidth, int windowHeight);
    void clear();
    
    void drawPoint(float x, float y, float size, const Color& color);
    void drawLine(float x1, float y1, float x2, float y2, float size, const Color& color);
    
    int getWidth() const { return textureWidth; }
    int getHeight() const { return textureHeight; }
    
    // Get canvas rectangle in window coordinates
    void getCanvasRect(int& x, int& y, int& w, int& h) const;
    
    // Convert window coordinates to canvas coordinates
    bool windowToCanvas(double windowX, double windowY, float& canvasX, float& canvasY) const;
    
private:
    void setupQuad();
    void setupBrushBuffers();
    void updateProjection();
    void saveViewport();
    void restoreViewport();
    
    int textureWidth;
    int textureHeight;
    int displayWidth;
    int displayHeight;
    int windowWidth;
    int windowHeight;
    GLint savedViewport[4];  // Saved viewport dimensions
    
    Color backgroundColor;
    glm::mat4 projection;    // Orthographic projection matrix
    float canvasScale;       // Scale factor for canvas display
    
    unsigned int frameBuffer;  // OpenGL framebuffer object
    unsigned int texture;      // OpenGL texture object
    unsigned int VAO, VBO, EBO;  // Canvas quad rendering objects
    unsigned int brushVAO, brushVBO;  // Brush rendering objects
    
    std::unique_ptr<Shader> shader;      // Canvas rendering shader
    std::unique_ptr<Shader> brushShader; // Brush rendering shader
};