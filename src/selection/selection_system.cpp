#include "selection_system.hpp"
#include "../core/canvas.hpp"
#include <glm/gtc/matrix_transform.hpp>

SelectionSystem::SelectionSystem(Canvas& canvas)
    : canvas(canvas), selectionMode(SelectionMode::None),
      frameBuffer(0), texture(0), VAO(0), VBO(0),
      startX(0), startY(0), lastX(0), lastY(0),
      commandManager(canvas.getCommandManager()) {
}

SelectionSystem::~SelectionSystem() {
    glDeleteFramebuffers(1, &frameBuffer);
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

bool SelectionSystem::init() {
    setupFramebuffer();
    setupShaders();
    setupBuffers();
    return true;
}

void SelectionSystem::setupShaders() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        
        uniform mat4 transform;
        uniform vec2 canvasSize;
        
        void main() {
            vec4 worldPos = transform * vec4(aPos, 0.0, 1.0);
            vec2 clipPos = (worldPos.xy / canvasSize) * 2.0 - 1.0;
            gl_Position = vec4(clipPos, 0.0, 1.0);
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        void main() {
            FragColor = vec4(0.2, 0.6, 1.0, 1.0); // 使用完全不透明的蓝色便于调试
        }
    )";
    
    shader = std::make_shared<Shader>(vertexShaderSource, fragmentShaderSource);
}


void SelectionSystem::setupFramebuffer() {
    // Create framebuffer
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 
                 canvas.getWidth(), canvas.getHeight(),
                 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                          GL_TEXTURE_2D, texture, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SelectionSystem::setupBuffers() {
    // Vertex data for rendering a quad
    float vertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SelectionSystem::beginSelection(float x, float y) {
    selectionMode = SelectionMode::Creating;
    startX = x;
    startY = y;
    lastX = x;
    lastY = y;
}

void SelectionSystem::updateSelection(float x, float y) {
    if (selectionMode != SelectionMode::Creating) return;

    int canvasHeight = canvas.getHeight();
    
    // Store the current selection rectangle
    float left = std::min(startX, x);
    float right = std::max(startX, x);
    float top = std::min(startY, y);
    float bottom = std::max(startY, y);
    
    selectionRect = PixelRect(
        static_cast<int>(left),
        static_cast<int>(top),
        static_cast<int>(right - left),
        static_cast<int>(bottom - top)
    );
}

void SelectionSystem::endSelection() {
    if (selectionMode != SelectionMode::Creating) return;
    
    // Capture the selection content
    Layer* activeLayer = canvas.getLayer(canvas.getActiveLayerIndex());
    std::cout << "Selection rect: x=" << selectionRect.x 
          << ", y=" << selectionRect.y 
          << ", w=" << selectionRect.width 
          << ", h=" << selectionRect.height << std::endl;
    if (activeLayer && selectionRect.width > 0 && selectionRect.height > 0) {
        selectionContent = activeLayer->getPixels(selectionRect);
        selectionMode = SelectionMode::Moving;
    } else {
        selectionMode = SelectionMode::None;
    }
}

void SelectionSystem::startMove(float x, float y) {
    if (!hasSelection()) return;
    
    selectionMode = SelectionMode::Moving;
    lastX = x;
    lastY = y;
}

void SelectionSystem::updateMove(float x, float y) {
    if (selectionMode != SelectionMode::Moving) return;
    
    float dx = x - lastX;
    float dy = y - lastY;
    
    transform.x += dx;
    transform.y += dy;
    
    lastX = x;
    lastY = y;
}

void SelectionSystem::endMove() {
    if (selectionMode != SelectionMode::Moving) return;
    
    selectionRect.x += static_cast<int>(transform.x);
    selectionRect.y += static_cast<int>(transform.y);
    
    transform = SelectionTransform();
}

void SelectionSystem::copySelection() {
    if (!hasSelection()) return;
    
    Layer* activeLayer = canvas.getLayer(canvas.getActiveLayerIndex());
    if (activeLayer) {
        PixelRect glRect = selectionRect;
        glRect.y = canvas.getHeight() - (selectionRect.y + selectionRect.height);
        clipboardContent = activeLayer->getPixels(glRect);
        clipboardRect = selectionRect;
    }
}

void SelectionSystem::cutSelection() {
    if (!hasSelection()) return;
    
    Layer* activeLayer = canvas.getLayer(canvas.getActiveLayerIndex());
    if (!activeLayer) return;

    PixelRect glRect = selectionRect;
    glRect.y = canvas.getHeight() - (selectionRect.y + selectionRect.height);
    
    // Save original content for undo
    std::vector<float> originalContent = activeLayer->getPixels(glRect);
    
    // Create cut command
    std::cout << "Creating cut command" << std::endl;
    auto command = std::make_unique<SelectionCommand>(
        activeLayer->getId(),
        glRect,
        originalContent,
        std::vector<float>(originalContent.size(), 0.0f)
    );
    
    // Save content to clipboard
    clipboardContent = originalContent;
    clipboardRect = selectionRect;
    
    // Execute command
    command->setCanvas(&canvas);
    command->execute();
    commandManager->addCommand(std::move(command));
}

void SelectionSystem::pasteSelection(float x, float y) {
    if (clipboardContent.empty()) return;

    Layer* activeLayer = canvas.getLayer(canvas.getActiveLayerIndex());
    if (!activeLayer) return;
    
    int canvasHeight = canvas.getHeight();
    float adjustedY = canvasHeight - y - clipboardRect.height;

    PixelRect pasteRect(
        std::clamp(static_cast<int>(x), 0, canvas.getWidth() - clipboardRect.width),
        std::clamp(static_cast<int>(adjustedY), 0, canvasHeight - clipboardRect.height),
        clipboardRect.width,
        clipboardRect.height
    );

    std::vector<float> originalContent = activeLayer->getPixels(pasteRect);

    auto command = std::make_unique<SelectionCommand>(
        activeLayer->getId(),
        pasteRect,
        originalContent,
        clipboardContent
    );

    command->setCanvas(&canvas);
    command->execute();
    commandManager->addCommand(std::move(command));
}


void SelectionSystem::deleteSelection() {
    if (!hasSelection()) return;
    
    Layer* activeLayer = canvas.getLayer(canvas.getActiveLayerIndex());
    if (!activeLayer) return;

    PixelRect glRect = selectionRect;
    glRect.y = canvas.getHeight() - (selectionRect.y + selectionRect.height);

    std::vector<float> originalContent = activeLayer->getPixels(glRect);

    auto command = std::make_unique<SelectionCommand>(
        activeLayer->getId(),
        glRect,
        originalContent,
        std::vector<float>(originalContent.size(), 0.0f)
    );

    command->setCanvas(&canvas);
    command->execute();
    commandManager->addCommand(std::move(command));

    selectionMode = SelectionMode::None;
    selectionContent.clear();
}

void SelectionSystem::render() {
    if (!hasSelection()) return;
    
    shader->use();
    
    // Save OpenGL state
    GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
    GLboolean depthTestWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLint previousLineWidth;
    glGetIntegerv(GL_LINE_WIDTH, &previousLineWidth);
    
    // Set shader uniforms
    glm::vec2 canvasSize(canvas.getWidth(), canvas.getHeight());
    shader->setVec2("canvasSize", canvasSize);
    
    // Calculate transform matrix
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, 
        glm::vec3(selectionRect.x, 
                  canvas.getHeight() - selectionRect.y - selectionRect.height, 
                  0.0f));
    transform = glm::scale(transform, 
        glm::vec3(selectionRect.width, selectionRect.height, 1.0f));
    
    shader->setMat4("transform", transform);
    
    // Enable blending
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Render selection content
    renderSelectionOutline();
    
    // Restore OpenGL state
    if(!blendWasEnabled) glDisable(GL_BLEND);
    if(depthTestWasEnabled) glEnable(GL_DEPTH_TEST);
    glLineWidth(previousLineWidth);
}

void SelectionSystem::renderSelectionOutline() {
    shader->use();
    
    glLineWidth(2.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_STRIP, 0, 5);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    
    glBindVertexArray(0);
}

void SelectionCommand::execute() {
    if (!canvas) return;
    
    Layer* layer = canvas->getLayerById(layerId);
    if (!layer) return;
    
    layer->setPixels(rect, newContent);
}

void SelectionCommand::undo() {
    if (!canvas) return;
    
    Layer* layer = canvas->getLayerById(layerId);
    if (!layer) return;
    
    layer->setPixels(rect, originalContent);
}