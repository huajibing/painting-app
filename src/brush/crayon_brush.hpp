#pragma once
#include "texture_brush.hpp"
#include <memory>
#include <random>

namespace CrayonBrush {

class CrayonBrush : public TextureBrush::TextureBrush {
public:
    CrayonBrush(unsigned int framebuffer, unsigned int texture, int width, int height)
        : TextureBrush(framebuffer, texture, width, height),
          grainSize(0.8f),
          paperRoughness(0.6f),
          pressureVariation(0.3f) {
    }
    
    void init() override {
        // Custom shader for crayon effect
        textureFragmentShader = R"(
            #version 330 core
            out vec4 FragColor;
            
            in vec2 TexCoord;
            in vec2 FragPos;
            
            uniform sampler2D brushTexture;
            uniform sampler2D layerTexture;
            uniform vec4 brushColor;
            uniform float grainSize;        // Size of grain texture
            uniform float paperRoughness;   // Paper texture influence
            uniform float pressureVariation;// Stroke pressure variation
            
            // Noise functions
            float random(vec2 st) {
                return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
            }
            
            float noise(vec2 st) {
                vec2 i = floor(st);
                vec2 f = fract(st);
                
                float a = random(i);
                float b = random(i + vec2(1.0, 0.0));
                float c = random(i + vec2(0.0, 1.0));
                float d = random(i + vec2(1.0, 1.0));
                
                vec2 u = f * f * (3.0 - 2.0 * f);
                return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
            }
            
            // Grain pattern
            float grainPattern(vec2 uv) {
                float n = noise(uv * grainSize * 50.0) * 0.5 + 0.5;
                return n * paperRoughness;
            }
            
            void main() {
                // Sample textures
                vec4 texColor = texture(brushTexture, FragPos);
                vec4 layerColor = texture(layerTexture, TexCoord);
                
                // Generate paper grain
                float grain = grainPattern(TexCoord);
                
                // Pressure variation based on noise
                float pressure = noise(FragPos * 10.0) * pressureVariation + (1.0 - pressureVariation);
                
                // Modify alpha based on grain and pressure
                float alpha = texColor.a * brushColor.a * pressure;
                alpha *= (1.0 - grain * 0.5); // Paper texture influence
                alpha = clamp(alpha, 0.0, 1.0);
                
                // Color blending with existing content
                vec3 finalColor;
                if (layerColor.a > 0.0) {
                    // Simple additive blending for crayon-like effect
                    finalColor = mix(layerColor.rgb, brushColor.rgb, alpha * 0.8);
                    
                    // Add slight color variation based on grain
                    finalColor += grain * 0.1 * brushColor.rgb;
                } else {
                    finalColor = brushColor.rgb;
                }
                
                // Apply grain texture to final color
                finalColor *= (1.0 - grain * 0.2);
                
                // Calculate final alpha
                float finalAlpha = max(alpha, layerColor.a);
                
                FragColor = vec4(finalColor, finalAlpha);
            }
        )";

        TextureBrush::init();
        
        // Load the crayon brush texture
        loadTexture("assets/brushes/crayon_brush.png");
        
        // Set brush parameters
        setRotationRange(0.0f, 360.0f);
        setScatterRange(0.0f, 1.0f);
        setRotationJitter(0.15f);
        setSizeJitter(0.1f);
        setSpacing(0.1f);
        
        // Crayon specific parameters
        opacityScale = 0.9f;
        minDistance = 0.05f;
    }
    
    void drawPoint(float x, float y, float size, const Color& color) override {
        brushShader->use();
        brushShader->setFloat("grainSize", grainSize);
        brushShader->setFloat("paperRoughness", paperRoughness);
        brushShader->setFloat("pressureVariation", pressureVariation);
        
        TextureBrush::drawPoint(x, y, size, color);
    }
    
    // Setters for crayon specific parameters
    void setGrainSize(float value) { grainSize = std::clamp(value, 0.0f, 1.0f); }
    void setPaperRoughness(float value) { paperRoughness = std::clamp(value, 0.0f, 1.0f); }
    void setPressureVariation(float value) { pressureVariation = std::clamp(value, 0.0f, 1.0f); }
    
private:
    float grainSize;         // Controls the size of the grain texture
    float paperRoughness;    // Controls how much the paper texture affects the stroke
    float pressureVariation; // Controls variation in stroke pressure
};

} // namespace CrayonBrush