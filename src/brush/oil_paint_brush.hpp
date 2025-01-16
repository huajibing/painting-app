#pragma once
#include "texture_brush.hpp"
#include <memory>
#include <random>

namespace OilPaintBrush {

class OilPaintBrush : public TextureBrush::TextureBrush {
public:
    OilPaintBrush(unsigned int framebuffer, unsigned int texture, int width, int height)
        : TextureBrush(framebuffer, texture, width, height),
          strokeDirection(0.0f),
          impasto(0.5f),
          mixingRatio(0.3f) {
    }
    
    void init() override {
        // Custom shader for oil paint effect
        textureFragmentShader = R"(
            #version 330 core
            out vec4 FragColor;
            
            in vec2 TexCoord;
            in vec2 FragPos;
            
            uniform sampler2D brushTexture;
            uniform sampler2D layerTexture;
            uniform vec4 brushColor;
            uniform float strokeDirection;  // Angle in radians
            uniform float impasto;         // Paint thickness
            uniform float mixingRatio;     // Color mixing strength
            
            // Noise function for texture variation
            float rand(vec2 co) {
                return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
            }
            
            // Function to create directional texture
            float directionalTexture(vec2 uv, float angle) {
                float c = cos(angle);
                float s = sin(angle);
                mat2 rotMat = mat2(c, -s, s, c);
                vec2 rotatedUV = rotMat * (uv - 0.5) + 0.5;
                return (sin(rotatedUV.x * 50.0) * 0.5 + 0.5) * impasto;
            }
            
            void main() {
                // Sample textures
                vec4 texColor = texture(brushTexture, FragPos);
                vec4 layerColor = texture(layerTexture, TexCoord);
                
                // Create directional texture based on stroke
                float dirTex = directionalTexture(FragPos, strokeDirection);
                
                // Add noise for paint texture variation
                float noise = rand(FragPos * 10.0) * 0.2;
                
                // Calculate base alpha
                float alpha = texColor.a * brushColor.a;
                
                // Modify alpha based on impasto and texture
                alpha *= (1.0 + dirTex + noise);
                alpha = clamp(alpha, 0.0, 1.0);
                
                // Color mixing with existing paint
                vec3 finalColor;
                if (layerColor.a > 0.0) {
                    // Mix colors based on thickness and existing paint
                    float mixFactor = mixingRatio * layerColor.a;
                    finalColor = mix(brushColor.rgb, layerColor.rgb, mixFactor);
                    
                    // Add slight color variation based on texture
                    finalColor += (dirTex * 0.1) * brushColor.rgb;
                } else {
                    finalColor = brushColor.rgb;
                }
                
                // Add thickness variation to final color
                finalColor *= (1.0 + dirTex * 0.2);
                
                // Calculate final alpha considering paint thickness
                float finalAlpha = max(alpha, layerColor.a);
                
                FragColor = vec4(finalColor, finalAlpha);
            }
        )";

        TextureBrush::init();
        
        // Load a custom brush texture for oil paint
        loadTexture("assets/brushes/oil_brush.png");
        
        // Set brush parameters
        setRotationRange(0.0f, 360.0f);
        setScatterRange(0.0f, 2.0f);
        setRotationJitter(0.2f);
        setSizeJitter(0.15f);
        setSpacing(0.1f);
        
        // Oil paint specific parameters
        opacityScale = 0.8f;
        minDistance = 0.05f;
    }
    
    void drawPoint(float x, float y, float size, const Color& color) override {
        // Update stroke direction based on movement
        if (isDrawing) {
            float dx = x - lastX;
            float dy = y - lastY;
            if (dx != 0.0f || dy != 0.0f) {
                strokeDirection = std::atan2(dy, dx);
            }
        }
        
        // Set oil paint specific uniforms
        brushShader->use();
        brushShader->setFloat("strokeDirection", strokeDirection);
        brushShader->setFloat("impasto", impasto);
        brushShader->setFloat("mixingRatio", mixingRatio);
        
        TextureBrush::drawPoint(x, y, size, color);
    }
    
    // Setters for oil paint specific parameters
    void setImpasto(float value) { impasto = std::clamp(value, 0.0f, 1.0f); }
    void setMixingRatio(float value) { mixingRatio = std::clamp(value, 0.0f, 1.0f); }
    
private:
    float strokeDirection;  // Current stroke direction
    float impasto;         // Paint thickness
    float mixingRatio;     // Color mixing strength
};

class RoughBrush : public OilPaintBrush {
public:
    RoughBrush(unsigned int framebuffer, unsigned int texture, int width, int height)
        : OilPaintBrush(framebuffer, texture, width, height) {
    }
    
    void init() override {
        OilPaintBrush::init();
        loadTexture("assets/brushes/rough_brush.png");
        opacityScale = 0.65f;
    }
};

} // namespace OilPaintBrush