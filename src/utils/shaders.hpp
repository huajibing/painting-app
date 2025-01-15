#pragma once
#include <string>

namespace Shaders {
    const std::string canvasVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        uniform mat4 projection;
        
        out vec2 TexCoord;
        
        void main() {
            gl_Position = projection * vec4(aPos.x, aPos.y, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    const std::string canvasFragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec2 TexCoord;
        uniform sampler2D layerTexture;
        uniform sampler2D compositeTexture;
        uniform float layerOpacity;
        uniform int previewMode;  // 0 for normal, 1 for erase
        
        void main() {
            vec4 layerColor = texture(layerTexture, TexCoord);
            vec4 compositeColor = texture(compositeTexture, TexCoord);
            
            if (previewMode == 1) {
                // Erase mode
                float eraseStrength = layerColor.a * layerOpacity;
                float finalAlpha = max(0.0, compositeColor.a * (1.0 - eraseStrength));
                FragColor = vec4(compositeColor.rgb, finalAlpha);
            } else {
                // Normal mode
                vec3 finalRGB = layerColor.rgb * layerColor.a * layerOpacity + 
                               compositeColor.rgb * compositeColor.a * (1.0 - layerColor.a * layerOpacity);
                float finalAlpha = layerColor.a * layerOpacity + compositeColor.a * (1.0 - layerColor.a * layerOpacity);
                FragColor = vec4(finalRGB / max(finalAlpha, 0.01), finalAlpha);
            }
        }
    )";

    const std::string brushVertexShader = R"(
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


    const std::string brushFragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec2 TexCoord;
        uniform sampler2D layerTexture;
        uniform vec4 brushColor;
        
        void main() {
            vec4 layerColor = texture(layerTexture, TexCoord);
            float mappedAlpha = brushColor.a; // pow(brushColor.a, 3) * 0.5;
            
            vec3 finalRGB = brushColor.rgb * mappedAlpha + layerColor.rgb * layerColor.a * (1.0 - mappedAlpha);
            float finalAlpha = mappedAlpha + layerColor.a * (1.0 - mappedAlpha);
            
            FragColor = vec4(finalRGB / max(finalAlpha, 0.01), finalAlpha);
        }
    )";

    const std::string mergeVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        out vec2 TexCoord;
        
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    const std::string mergeFragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec2 TexCoord;
        uniform sampler2D layerTexture;
        uniform sampler2D strokeTexture;
        uniform int strokeMode; // 0 for normal, 1 for erase
        
        void main() {
            vec4 layerColor = texture(layerTexture, TexCoord);
            vec4 strokeColor = texture(strokeTexture, TexCoord);
            
            if (strokeMode == 1) { // Erase mode
                float eraseStrength = strokeColor.a;
                float finalAlpha = max(0.0, layerColor.a * (1.0 - eraseStrength));
                FragColor = vec4(layerColor.rgb, finalAlpha);
            }
            else { // Normal mode
                vec3 finalRGB = strokeColor.rgb * strokeColor.a + 
                               layerColor.rgb * layerColor.a * (1.0 - strokeColor.a);
                float finalAlpha = strokeColor.a + layerColor.a * (1.0 - strokeColor.a);
                FragColor = vec4(finalRGB / max(finalAlpha, 0.01), finalAlpha);
            }
        }
    )";
}