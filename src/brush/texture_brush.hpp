#pragma once
#include "brush.hpp"
#include "../utils/shader.hpp"
#include <glm/glm.hpp>
#include <random>

namespace TextureBrush{

    class TextureBrush : public Brush {
    public:
        TextureBrush(unsigned int framebuffer, unsigned int texture, int width, int height);
        ~TextureBrush();
        
        virtual void init() override;
        void loadTexture(const std::string& path);
        
        void drawPoint(float x, float y, float size, const Color& color) override;
        std::vector<std::vector<float>> drawLine(float x1, float y1, 
                                            float x2, float y2,
                                            float size, const Color& color) override;
        
        virtual void beginStroke() override;
        virtual void endStroke() override;
        
        // Brush settings
        void setRotationRange(float min, float max) { 
            minRotation = min; 
            maxRotation = max; 
        }
        void setScatterRange(float min, float max) {
            minScatter = min;
            maxScatter = max;
        }
        void setRotationJitter(float jitter) { rotationJitter = jitter; }
        void setSizeJitter(float jitter) { sizeJitter = jitter; }
        void setSpacing(float s) { spacing = s; }
        
    protected:
        unsigned int frameBuffer;
        unsigned int texture;
        unsigned int brushVAO;
        unsigned int brushVBO;
        unsigned int brushTexture;
        std::shared_ptr<Shader> brushShader;
        int width, height;
        int vertexCount;
        int lastX, lastY;
        bool isDrawing = false;
        
        // Brush parameters
        float minRotation = 0.0f;
        float maxRotation = 360.0f;
        float minScatter = 0.0f;
        float maxScatter = 0.0f;
        float rotationJitter = 0.0f;
        float sizeJitter = 0.0f;
        float spacing = 0.25f;
        float minDistance = 0.2f;
        float sizeScale = 1.0f;
        float opacityScale = 1.0f;
        
        // Random number generation
        std::mt19937 rng;
        std::uniform_real_distribution<float> rotationDist;
        std::uniform_real_distribution<float> scatterDist;
        
        // Shader sources
        std::string textureVertexShader = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec2 aTexCoord;
            
            uniform vec2 position;
            uniform float size;
            uniform float rotation;
            uniform float aspectRatio;
            
            out vec2 TexCoord;
            out vec2 FragPos;
            
            void main() {
                // Rotate the vertex
                float rad = radians(rotation);
                mat2 rotMat = mat2(
                    cos(rad), -sin(rad),
                    sin(rad), cos(rad)
                );
                
                // Apply transformations
                vec2 pos = aPos * size;
                pos.x *= aspectRatio;
                pos += position;
                
                gl_Position = vec4(pos, 0.0, 1.0);
                TexCoord = pos * 0.5 + vec2(0.5);
                FragPos = 0.5 * (rotMat * aPos + vec2(1.0));
            }
        )";

        std::string textureFragmentShader = R"(
            #version 330 core
            out vec4 FragColor;
            
            in vec2 TexCoord;
            in vec2 FragPos;
            
            uniform sampler2D brushTexture;
            uniform vec4 brushColor;
            uniform sampler2D layerTexture;
            
            void main() {
                vec4 texColor = texture(brushTexture, FragPos);
                vec4 layerColor = texture(layerTexture, TexCoord);
                
                // Multiply texture alpha by brush color alpha
                float alpha = texColor.a * brushColor.a;
                
                // Blend with existing content
                vec3 finalRGB = brushColor.rgb * alpha + 
                            layerColor.rgb * layerColor.a * (1.0 - alpha);
                float finalAlpha = alpha + layerColor.a * (1.0 - alpha);
                
                FragColor = vec4(finalRGB / max(finalAlpha, 0.001), finalAlpha);
            }
        )";
        
        virtual std::vector<float> getVertices();
        float getRandomRotation();
        glm::vec2 getRandomScatter();
        float getRandomSizeMultiplier();
    };


    class PencilBrush : public TextureBrush {
    public:
        PencilBrush(unsigned int framebuffer, unsigned int texture, int width, int height)
            : TextureBrush(framebuffer, texture, width, height) {
        }
        
        void init() override {
            TextureBrush::init();
            loadTexture("assets/brushes/rough_brush.png");
            setRotationRange(0.0f, 360.0f);
            setScatterRange(0.0f, 0.1f);
            setRotationJitter(0.1f);
            setSizeJitter(0.1f);
            setSpacing(0.1f);
            minDistance = 0.8f;
            sizeScale = 0.025f;
        }
    };

    class AcrylicBrush : public TextureBrush {
    public:
        AcrylicBrush(unsigned int framebuffer, unsigned int texture, int width, int height)
            : TextureBrush(framebuffer, texture, width, height) {
        }
        
        void init() override {
            textureFragmentShader = R"(
                #version 330 core
                out vec4 FragColor;
                
                in vec2 TexCoord;
                in vec2 FragPos;
                
                uniform sampler2D brushTexture;
                uniform vec4 brushColor;
                uniform sampler2D layerTexture;
                
                void main() {
                    vec4 texColor = texture(brushTexture, FragPos);
                    vec4 layerColor = texture(layerTexture, TexCoord);
                    
                    // Multiply texture alpha by brush color alpha
                    float alpha = texColor.a * brushColor.a;
                    
                    // Blend with existing content
                    vec3 finalRGB = brushColor.rgb * alpha + 
                                layerColor.rgb * layerColor.a * (1.0 - alpha);
                    float finalAlpha = max(alpha, layerColor.a);
                    
                    FragColor = vec4(finalRGB / max(finalAlpha, 0.01), finalAlpha);
                }
            )";
            TextureBrush::init();
            loadTexture("assets/brushes/scatter_brush.png");
            setRotationRange(0.0f, 0.0f);
            setScatterRange(0.0f, 0.1f);
            setRotationJitter(0.1f);
            setSizeJitter(0.1f);
            setSpacing(0.02f);
            minDistance = 0.0f;
        }
    };

    class SoftCircleBrush : public TextureBrush {
    public:
        SoftCircleBrush(unsigned int framebuffer, unsigned int texture, int width, int height)
            : TextureBrush(framebuffer, texture, width, height) {
        }
        
        void init() override {
            TextureBrush::init();
            loadTexture("assets/brushes/circle_soft.png");
            setRotationRange(0.0f, 360.0f);
            setScatterRange(0.0f, 0.1f);
            setRotationJitter(0.1f);
            setSizeJitter(0.1f);
            setSpacing(0.1f);
            minDistance = 0.1f;
            opacityScale = 0.3f;
        }
    };
} // namespace TextureBrush