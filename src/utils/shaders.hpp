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
        uniform sampler2D canvasTexture;
        
        void main() {
            FragColor = texture(canvasTexture, TexCoord);
        }
    )";

    const std::string brushVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        
        uniform vec2 position;
        uniform float size;
        uniform float aspectRatio;
        
        out vec2 TexCoord;
        
        void main() {
            vec2 offset = aPos * size;
            offset.x *= aspectRatio;
            
            vec2 finalPosition = position + offset;
            
            gl_Position = vec4(finalPosition, 0.0, 1.0);
            
            TexCoord = (finalPosition + 1.0) * 0.5;
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
            float mappedAlpha = pow(brushColor.a, 3) * 0.5;
            
            vec3 finalRGB = brushColor.rgb * mappedAlpha + layerColor.rgb * layerColor.a * (1.0 - mappedAlpha);
            float finalAlpha = mappedAlpha + layerColor.a * (1.0 - mappedAlpha);
            
            FragColor = vec4(finalRGB / max(finalAlpha, 0.01), finalAlpha);
        }
    )";
}