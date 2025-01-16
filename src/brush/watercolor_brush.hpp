#pragma once
#include "texture_brush.hpp"
#include <memory>

namespace WatercolorBrush {

class WatercolorBrush : public TextureBrush::TextureBrush {
public:
    WatercolorBrush(unsigned int framebuffer, unsigned int texture, int width, int height)
        : TextureBrush(framebuffer, texture, width, height) {
    }
    
    void init() override {
        // Custom shader for watercolor effect
        textureFragmentShader = R"(
            #version 330 core
            out vec4 FragColor;
            
            in vec2 TexCoord;
            in vec2 FragPos;
            
            uniform sampler2D brushTexture;
            uniform sampler2D layerTexture;
            uniform vec4 brushColor;
            uniform float wetness;      // Controls color spread
            uniform float granulation;  // Controls texture granulation
            
            // Noise function for granulation effect
            float rand(vec2 co) {
                return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
            }
            
            void main() {
                // Sample textures
                vec4 texColor = texture(brushTexture, FragPos);
                vec4 layerColor = texture(layerTexture, TexCoord);
                
                // Add granulation effect
                float noise = rand(FragPos * 100.0) * granulation;
                float alpha = texColor.a * brushColor.a;
                
                // Modify alpha based on wetness and noise
                alpha = alpha * (1.0 - wetness * 0.5) + noise * wetness;
                alpha = clamp(alpha, 0.0, 1.0);
                
                // Color blending with existing content
                vec3 finalColor;
                if (layerColor.a > 0.0) {
                    // Wet-on-wet blending
                    float blend = wetness * 0.8;
                    finalColor = mix(brushColor.rgb, layerColor.rgb, blend) * alpha +
                                layerColor.rgb * layerColor.a * (1.0 - alpha);
                } else {
                    // Wet-on-dry
                    finalColor = brushColor.rgb * alpha;
                }
                
                float finalAlpha = max(alpha, layerColor.a);
                
                // Edge darkening effect
                float edgeDarken = 1.0 - smoothstep(0.2, 0.8, texColor.a);
                finalColor *= (1.0 - edgeDarken * 0.2);
                
                FragColor = vec4(finalColor, finalAlpha);
            }
        )";

        TextureBrush::init();
        
        // Load a soft, watercolor-like texture
        loadTexture("assets/brushes/circle_soft.png");
        
        // Set brush parameters
        setRotationRange(0.0f, 360.0f);
        setScatterRange(0.0f, 2.0f);    // Increased scatter for natural look
        setRotationJitter(0.2f);        // More rotation variation
        setSizeJitter(0.15f);           // Size variation
        setSpacing(0.15f);              // Closer spacing for smooth strokes
        
        // Additional watercolor-specific parameters
        wetness = 0.2f;                 // Default wetness
        granulation = 0.5f;             // Default granulation
        opacityScale = 0.6f;            // More transparent for watercolor look
        minDistance = 0.1f;             // Allow close stamps for smooth strokes
    }
    
    void drawPoint(float x, float y, float size, const Color& color) override {
        brushShader->use();
        brushShader->setFloat("wetness", wetness);
        brushShader->setFloat("granulation", granulation);
        
        TextureBrush::drawPoint(x, y, size, color);
    }
    
    // Setters for watercolor-specific parameters
    void setWetness(float value) { wetness = std::clamp(value, 0.0f, 1.0f); }
    void setGranulation(float value) { granulation = std::clamp(value, 0.0f, 1.0f); }
    
private:
    float wetness;     // Controls how much the color spreads and blends
    float granulation; // Controls the texture granulation effect
};

} // namespace WatercolorBrush