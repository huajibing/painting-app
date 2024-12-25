#pragma once
#include "../utils/color.hpp"
#include <memory>
#include <vector>
#include <string>
#include "brush.hpp"
#include "../utils/shader.hpp"

namespace BaseBrush {

    class BaseBrush : public Brush {
    public:
        BaseBrush(unsigned int framebuffer,
                int width, int height);
        ~BaseBrush();
        
        void init() override;
        
        virtual std::vector<float> getVertices() = 0;
        void drawPoint(float x, float y,
                        float size, const Color& color) override;
        std::vector<std::vector<float>> drawLine(
            float x1, float y1,
            float x2, float y2, 
            float size, const Color& color) override;

        void beginStroke() override {}
        void endStroke() override {}

    private:
        unsigned int frameBuffer;
        unsigned int brushVAO;
        unsigned int brushVBO;
        std::shared_ptr<Shader> brushShader;
        int width, height;
        int vertexCount;
        std::string baseVertexShader = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            
            uniform vec2 position;
            uniform float size;
            uniform float aspectRatio;
            
            void main() {
                vec2 pos = aPos * size;
                pos.x *= aspectRatio;
                pos += position;
                gl_Position = vec4(pos, 0.0, 1.0);
            }
        )";

        std::string baseFragmentShader = R"(
            #version 330 core
            out vec4 FragColor;
            
            in vec2 TexCoord;
            uniform sampler2D layerTexture;
            uniform vec4 brushColor;
            
            void main() {
                vec4 layerColor = texture(layerTexture, TexCoord);
                float mappedAlpha = brushColor.a;
                
                vec3 finalRGB = brushColor.rgb * mappedAlpha + layerColor.rgb * layerColor.a * (1.0 - mappedAlpha);
                float finalAlpha = mappedAlpha + layerColor.a * (1.0 - mappedAlpha);
                
                FragColor = vec4(finalRGB / max(finalAlpha, 0.01), finalAlpha);
            }
        )";
    };

    class CircleBrush : public BaseBrush {
    public:
        CircleBrush(unsigned int framebuffer,
                    int width, int height)
                    : BaseBrush(framebuffer, width, height) {};
        std::vector<float> getVertices() override;
    };

    class SquareBrush : public BaseBrush {
    public:
        SquareBrush(unsigned int framebuffer,
                    int width, int height)
                    : BaseBrush(framebuffer, width, height) {};
        std::vector<float> getVertices() override;
    };

} // namespace BaseBrush