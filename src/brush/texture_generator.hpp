#include <vector>
#include <string>
#include <cmath>
#include "stb_image_write.h"
#define PI 3.14159265359

class BrushTextureGenerator {
public:
    std::vector<unsigned char> generateScatterBrush(int size, int numPoints = 200) {
        std::vector<unsigned char> data(size * size * 4, 0);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> radiusDist(0.0f, 1.0f);
        std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * PI);
        std::normal_distribution<float> sizeDist(4.0f, 1.5f);
        
        for(int i = 0; i < numPoints; i++) {
            float r = std::sqrt(radiusDist(gen)) * size * 0.45f;
            float angle = angleDist(gen);
            
            int x = static_cast<int>(size/2 + r * std::cos(angle));
            int y = static_cast<int>(size/2 + r * std::sin(angle));
            
            float pointSize = std::max(1.0f, sizeDist(gen));
            
            drawGaussianPointRGBA(data, size, x, y, pointSize);
        }
        
        return data;
    }

    std::vector<unsigned char> generateRoughBrush(int size, 
        float roughness = 0.7f,
        float noiseScale = 30.0f,
        int numHoles = 12) {
        
        std::vector<unsigned char> data(size * size * 4, 0);
        std::random_device rd;
        std::mt19937 gen(rd());

        std::vector<glm::vec2> controlPoints;
        int numPoints = 64;
        float baseRadius = size * 0.45f;
        generateControlPoints(controlPoints, numPoints, baseRadius, roughness, gen);

        std::vector<float> noiseMap = generateNoiseMap(size, noiseScale);

        std::vector<Hole> holes;
        generateHoles(holes, numHoles, size, gen);

        for(int y = 0; y < size; y++) {
            for(int x = 0; x < size; x++) {

                float px = (x - size/2.0f);
                float py = (y - size/2.0f);
                glm::vec2 point(px, py);

                bool inside = isPointInShape(point, controlPoints);

                for(const auto& hole : holes) {
                    if(isPointInHole(point, hole)) {
                        inside = false;
                        break;
                    }
                }
                
                if(inside) {
                    int idx = (y * size + x) * 4;

                    float noiseValue = noiseMap[y * size + x];
                    float alpha = std::max(std::min(1.0f, 0.5f + noiseValue * 0.6f), 0.0f);

                    data[idx + 0] = 255; // R
                    data[idx + 1] = 255; // G
                    data[idx + 2] = 255; // B
                    data[idx + 3] = static_cast<unsigned char>(alpha * 255);
                }
            }
        }
        
        return data;
    }

    
private:
    void drawGaussianPointRGBA(std::vector<unsigned char>& data, int size, 
                              int centerX, int centerY, float pointSize) {
        int radius = static_cast<int>(pointSize * 2.5f);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> alpha(0.4f, 0.8f);
        float alphaScale = alpha(gen);
        
        for(int y = -radius; y <= radius; y++) {
            for(int x = -radius; x <= radius; x++) {
                int px = centerX + x;
                int py = centerY + y;
                
                if(px >= 0 && px < size && py >= 0 && py < size) {
                    float dist = std::sqrt(x*x + y*y);
                    float intensity = std::min(1.0f, std::max(0.0f, (1.0f - std::pow(dist / float(radius), 2.0f))) * std::exp(-pointSize * pointSize/20.0f));
                    
                    int idx = (py * size + px) * 4;
                    
                    data[idx + 0] = 255;
                    data[idx + 1] = 255;
                    data[idx + 2] = 255;

                    unsigned char currentAlpha = data[idx + 3];
                    unsigned char newAlpha = static_cast<unsigned char>(intensity * 255);
                    data[idx + 3] = std::min(255u, 
                        std::max(static_cast<unsigned int>(currentAlpha), static_cast<unsigned int>(newAlpha)));
                }
            }
        }
    }

    struct Hole {
        glm::vec2 center;
        float radius;
        std::vector<glm::vec2> points;
    };
    

    void generateControlPoints(std::vector<glm::vec2>& points, 
                             int numPoints, float baseRadius,
                             float roughness, std::mt19937& gen) {
        std::uniform_real_distribution<float> radiusDist(-roughness, roughness);
        
        for(int i = 0; i < numPoints; i++) {
            float angle = (i * 2.0f * PI) / numPoints;

            float radius = baseRadius * (1.0f + radiusDist(gen));
            float x = std::cos(angle) * radius;
            float y = std::sin(angle) * radius;
            points.push_back(glm::vec2(x, y));
        }
    }
    
    std::vector<float> generateNoiseMap(int size, float scale) {
        std::vector<float> noiseMap(size * size);
        
        for(int y = 0; y < size; y++) {
            for(int x = 0; x < size; x++) {
                float nx = x / static_cast<float>(size);
                float ny = y / static_cast<float>(size);

                float noise = 0.0f;
                float amplitude = 1.0f;
                float frequency = 1.0f;
                
                for(int i = 0; i < 4; i++) {
                    noise += amplitude * perlinNoise(nx * frequency * scale, 
                                                   ny * frequency * scale);
                    amplitude *= 0.5f;
                    frequency *= 2.0f;
                }

                noise = (noise + 1.0f) * 0.5f;
                noiseMap[y * size + x] = noise;
            }
        }
        
        return noiseMap;
    }

    void generateHoles(std::vector<Hole>& holes, int numHoles, 
                      int size, std::mt19937& gen) {
        std::uniform_real_distribution<float> posDist(-0.5f, 0.5f);
        std::uniform_real_distribution<float> sizeDist(0.0f, 0.10f);
        
        for(int i = 0; i < numHoles; i++) {
            Hole hole;
            hole.center = glm::vec2(posDist(gen) * size, posDist(gen) * size);
            hole.radius = sizeDist(gen) * size;

            int numPoints = 16;
            std::uniform_real_distribution<float> radiusDist(0.8f, 1.2f);
            
            for(int j = 0; j < numPoints; j++) {
                float angle = (j * 2.0f * PI) / numPoints;
                float radius = hole.radius * radiusDist(gen);
                float x = hole.center.x + std::cos(angle) * radius;
                float y = hole.center.y + std::sin(angle) * radius;
                hole.points.push_back(glm::vec2(x, y));
            }
            
            holes.push_back(hole);
        }
    }

    bool isPointInShape(const glm::vec2& point, 
                       const std::vector<glm::vec2>& vertices) {
        bool inside = false;
        int j = vertices.size() - 1;
        
        for(int i = 0; i < vertices.size(); i++) {
            if((vertices[i].y > point.y) != (vertices[j].y > point.y) &&
               point.x < (vertices[j].x - vertices[i].x) * 
               (point.y - vertices[i].y) / 
               (vertices[j].y - vertices[i].y) + vertices[i].x) {
                inside = !inside;
            }
            j = i;
        }
        
        return inside;
    }

    bool isPointInHole(const glm::vec2& point, const Hole& hole) {
        return isPointInShape(point, hole.points);
    }

    float perlinNoise(float x, float y) {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;
        x -= std::floor(x);
        y -= std::floor(y);
        
        float u = fade(x);
        float v = fade(y);
        
        int A = P[X] + Y;
        int B = P[(X + 1) & 255] + Y;
        
        return lerp(v, 
            lerp(u, grad(P[A], x, y), grad(P[B], x-1, y)),
            lerp(u, grad(P[A+1], x, y-1), grad(P[B+1], x-1, y-1)));
    }
    
    float fade(float t) { 
        return t * t * t * (t * (t * 6 - 15) + 10); 
    }
    
    float lerp(float t, float a, float b) { 
        return a + t * (b - a); 
    }
    
    float grad(int hash, float x, float y) {
        int h = hash & 7;
        float u = h < 4 ? x : y;
        float v = h < 4 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
    }

    const int P[512] = {
        151,160,137,91,90,15,					
        131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,	
        190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
        88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
        77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
        102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
        135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
        5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
        223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
        129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
        251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
        49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
        138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,

        151,160,137,91,90,15,					
        131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,	
        190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
        88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
        77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
        102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
        135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
        5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
        223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
        129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
        251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
        49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
        138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
    };
};