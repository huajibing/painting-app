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
        
        out vec2 FragPos;
        
        void main() {
            vec2 offset = aPos * size;
            offset.x *= aspectRatio;
            
            vec2 finalPosition = position + offset;
            
            gl_Position = vec4(finalPosition, 0.0, 1.0);
            
            FragPos = aPos;
        }
    )";
    const std::string brushFragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        
        uniform vec4 brushColor;
        
        in vec2 FragPos;

        void main() {
            float dist = length(FragPos);

            // Opacity mapping
            float finalAlpha = pow(brushColor.a, 3.0) * 0.5;
            
            FragColor = vec4(brushColor.rgb, finalAlpha);
        }
    )";
}