#pragma once
#include <imgui.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <nfd.hpp>
#include "../brush/brush_system.hpp"
#include "../core/canvas.hpp"
#include "../file/file_system.hpp"
#include "cursor_manager.hpp"

#define ICON_MIN_FA 0xe005
#define ICON_MAX_FA 0xf8ff

#define ICON_FA_PENCIL "\xef\x8c\x83"          // f303
#define ICON_FA_ERASER "\xef\x84\xad"          // f12d
#define ICON_FA_FILL "\xef\x95\x75"            // f575
#define ICON_FA_CROP "\xef\x84\xa5"            // f125
#define ICON_FA_LAYER_GROUP "\xef\x97\xbd"     // f5fd
#define ICON_FA_EYE "\xef\x81\xae"             // f06e
#define ICON_FA_EYE_SLASH "\xef\x81\xb0"       // f070
#define ICON_FA_BRUSH "\xef\x95\x9d"           // f55d
#define ICON_FA_PALETTE "\xef\x94\xbf"         // f53f
#define ICON_FA_FILE "\xef\x85\x9b"           // f15b
#define ICON_FA_FOLDER_OPEN "\xef\x81\xbc"    // f07c
#define ICON_FA_SAVE "\xef\x83\x87"           // f0c7
#define ICON_FA_UNDO "\xef\x83\xa2"           // f0e2
#define ICON_FA_REDO "\xef\x80\x9e"           // f01e
#define ICON_FA_PLUS "\xef\x81\xa7"           // f067
#define ICON_FA_GEAR "\xef\x80\x93"           // f013
#define ICON_FA_TRASH "\xef\x87\xb8"          // f2ed
#define ICON_FA_ARROW_POINTER "\xef\x89\x85"  // f245
#define ICON_FA_GRIP_LINES "\xef\x9e\xa4"     // f7a4
#define ICON_FA_FILE_EXPORT "\xef\x95\xae"    // f56e
#define ICON_FA_QUESTION_CIRCLE "\xef\x81\x99" // f059
#define ICON_FA_BOOK "\xef\x80\xad"           // f02d
#define ICON_FA_KEYBOARD "\xef\x84\x9c"        // f11c
#define ICON_FA_TOOLBOX "\xef\x95\x92"        // f552
#define ICON_FA_XMARK "\xef\x80\x8d"          // f00d
#define ICON_FA_GAMEPAD "\xef\x84\x9b"        // f11b

class UIManager {
public:
    UIManager();
    ~UIManager() { cleanup(); }
    
    bool init();
    void render();
    bool shouldClose() const;
    
    void setBrushSystem(BrushSystem* bs);
    void setCanvas(Canvas* c);
    void setFileSystem(FileSystem* fs) { fileSystem = fs; }
    
    GLFWwindow* getWindow() const { return window; }
    // Canvas coordinate conversion
    bool windowToCanvas(double windowX, double windowY, float& canvasX, float& canvasY) const {
        if (!canvas) return false;
        
        // Check if the point is within the canvas area
        if (windowX < canvasDisplayPos.x || windowX >= canvasDisplayPos.x + canvasDisplaySize.x ||
            windowY < canvasDisplayPos.y || windowY >= canvasDisplayPos.y + canvasDisplaySize.y) {
            return false;
        }

        // Convert to canvas coordinates
        float normalizedX = (windowX - canvasDisplayPos.x) / canvasDisplaySize.x;
        float normalizedY = (windowY - canvasDisplayPos.y) / canvasDisplaySize.y;
        
        canvasX = normalizedX * canvas->getWidth();
        canvasY = normalizedY * canvas->getHeight();
        
        return true;
    }

    const float* getBrushColor() const { return brushColor; }
    float getBrushSize() const { return brushSize; }
    float getBrushOpacity() const { return brushOpacity; }
    void updateBrushColor(float r, float g, float b) {
        brushColor[0] = r;
        brushColor[1] = g;
        brushColor[2] = b;
    }
    void updateBrushSize(float size) { brushSize = size; }

    void showOpenDialog();
    void showSaveDialog();
    void handleExportImage(const std::string& format);
    void handleNewFile();

    void renderMainMenuBar(float height);
    void renderLeftToolbar(float width, float yOffset, float height);
    void renderRightPanel(float x, float y, float width, float height);
    void renderLayersList();
    void renderCanvasArea(float x, float y, float width, float height);

    void enableGamepadCursor(bool enable) { hasGamepadCursor = enable; }
    void updateGamepadCursor(float x, float y);
    void setGamepadName(const std::string& name) { gamepadName = name; }

private:
    void cleanup();
    void setupStyle();

    void showExportDialog(const std::string& defaultFormat);
    static std::string getFileExtension(const std::string& path);
    static bool addDefaultExtension(std::string& path, const std::string& ext);
    
    GLFWwindow* window;
    BrushSystem* brushSystem;
    Canvas* canvas;
    FileSystem* fileSystem;
    std::unique_ptr<CursorManager> cursorManager;
    
    // UI state
    float brushColor[3];
    float brushSize;
    float brushOpacity;
    bool sidebarVisible;
    
    // Canvas display info
    ImVec2 canvasDisplayPos;
    ImVec2 canvasDisplaySize;
    
    // Cache last frame state
    ImVec2 lastWindowSize;
    bool lastFrameValid = false;

    // Fonts
    ImFont* regularFont;
    ImFont* largeRegularFont;
    ImFont* boldFont;
    ImFont* largeBoldFont;
    ImFont* iconFont;
    ImFont* largeIconFont;

    void renderHelpPopup();
    void renderCoordinateOverlay();
    bool showHelpPopup = false;
    ImVec2 currentMousePos{0, 0};

    void renderGamepadCursor();
    void setGamepadCursorPosition(float x, float y) { 
        gamepadCursorX = x;
        gamepadCursorY = y;
    }
    bool hasGamepadCursor = false;
    float gamepadCursorX = 0.0f;
    float gamepadCursorY = 0.0f;
    std::string gamepadName = "";
};