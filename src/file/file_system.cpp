#include "file_system.hpp"
#include <fstream>
#include <iostream>
#include "stb_image.h"
#include "stb_image_write.h"

FileSystem::FileSystem(Canvas& canvas) : canvas(canvas) {}

bool FileSystem::importImage(const std::string& path, bool asNewLayer) {
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    
    if (!data) {
        std::cerr << "Failed to load image: " << path << std::endl;
        return false;
    }
    
    // Target layer or new layer
    Layer* targetLayer;
    if (asNewLayer) {
        canvas.addLayer("Imported Image");
        targetLayer = canvas.getLayer(canvas.getLayerCount() - 1);
    } else {
        targetLayer = canvas.getLayer(canvas.getActiveLayerIndex());
    }
    
    if (!targetLayer) {
        stbi_image_free(data);
        return false;
    }
    
    // Convert pixel data to float
    std::vector<float> pixelData(width * height * 4);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int srcIndex = (y * width + x) * 4;
            int dstIndex = ((height - 1 - y) * width + x) * 4;

            pixelData[dstIndex + 0] = data[srcIndex + 0] / 255.0f;
            pixelData[dstIndex + 1] = data[srcIndex + 1] / 255.0f;
            pixelData[dstIndex + 2] = data[srcIndex + 2] / 255.0f;
            pixelData[dstIndex + 3] = data[srcIndex + 3] / 255.0f;
        }
    }
    
    stbi_image_free(data);
    
    // Set layer pixels
    PixelRect rect{0, 0, width, height};
    try {
        targetLayer->setPixels(rect, pixelData);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to set layer pixels: " << e.what() << std::endl;
        return false;
    }
}

bool FileSystem::exportImage(const std::string& path, bool mergeVisible) {
    if (mergeVisible) {
        unsigned int textureID = canvas.getCompositeTexture();
        int width = canvas.getWidth();
        int height = canvas.getHeight();
        
        // Read pixel data from texture
        std::vector<float> pixelData(width * height * 4);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixelData.data());
        
        // Convert to 8-bit and flip image
        std::vector<unsigned char> outputData(width * height * 4);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int srcIndex = ((height - 1 - y) * width + x) * 4;
                int dstIndex = (y * width + x) * 4;
                
                float r = pixelData[srcIndex + 0];
                float g = pixelData[srcIndex + 1];
                float b = pixelData[srcIndex + 2];
                float a = pixelData[srcIndex + 3];

                if (a > 0) {
                    r /= a;
                    g /= a;
                    b /= a;
                }

                outputData[dstIndex + 0] = static_cast<unsigned char>(std::min(255.0f, std::max(0.0f, r * 255.0f)));
                outputData[dstIndex + 1] = static_cast<unsigned char>(std::min(255.0f, std::max(0.0f, g * 255.0f)));
                outputData[dstIndex + 2] = static_cast<unsigned char>(std::min(255.0f, std::max(0.0f, b * 255.0f)));
                outputData[dstIndex + 3] = static_cast<unsigned char>(std::min(255.0f, std::max(0.0f, a * 255.0f)));
            }
        }
        
        // Save image
        std::string extension = path.substr(path.find_last_of(".") + 1);
        if (extension == "png") {
            return stbi_write_png(path.c_str(), width, height, 4, 
                                outputData.data(), width * 4);
        } else if (extension == "jpg" || extension == "jpeg") {
            return stbi_write_jpg(path.c_str(), width, height, 4, 
                                outputData.data(), 95);
        }
        return false;
    } else {
        // Get active layer
        Layer* layer = canvas.getLayer(canvas.getActiveLayerIndex());
        if (!layer) return false;
        
        // Get pixel data
        PixelRect rect{0, 0, canvas.getWidth(), canvas.getHeight()};
        std::vector<float> pixelData = layer->getPixels(rect);
        
        // Convert to 8-bit
        std::vector<unsigned char> outputData;
        outputData.reserve(pixelData.size());
        
        for (float value : pixelData) {
            outputData.push_back(static_cast<unsigned char>(value * 255.0f));
        }
        
        // Save image
        std::string extension = path.substr(path.find_last_of(".") + 1);
        bool success = false;
        
        if (extension == "png") {
            success = stbi_write_png(path.c_str(), canvas.getWidth(), canvas.getHeight(),
                                4, outputData.data(), canvas.getWidth() * 4);
            std::cout << "png" << std::endl;
        } else if (extension == "jpg" || extension == "jpeg") {
            success = stbi_write_jpg(path.c_str(), canvas.getWidth(), canvas.getHeight(),
                                4, outputData.data(), 95);
        }
        
        return success;
    }
}

bool FileSystem::saveProject(const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    
    // Write project header
    ProjectHeader header;
    header.magic[0] = 'P'; header.magic[1] = 'A';
    header.magic[2] = 'I'; header.magic[3] = 'N';
    header.version = 1;
    header.width = canvas.getWidth();
    header.height = canvas.getHeight();
    header.numLayers = canvas.getLayerCount();
    header.flags = 0;
    
    if (!writeProjectHeader(file, header)) {
        return false;
    }
    
    // Save each layer
    for (size_t i = 0; i < canvas.getLayerCount(); ++i) {
        Layer* layer = canvas.getLayer(i);
        if (!layer) continue;
        
        if (!writeLayerInfo(file, layer)) {
            return false;
        }
        
        if (!writeLayerData(file, layer)) {
            return false;
        }
    }
    
    return true;
}

bool FileSystem::loadProject(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    
    // Read project header
    ProjectHeader header;
    if (!readProjectHeader(file, header)) {
        return false;
    }
    
    if (!validateProjectHeader(header)) {
        return false;
    }
    
    // Clear existing layers
    for (size_t i = 0; i < canvas.getLayerCount(); ++i) {
        canvas.removeLayer(i, false);
    }
    
    // Set up layers
    for (uint32_t i = 0; i < header.numLayers; ++i) {
        LayerInfo layerInfo;
        if (!readLayerInfo(file, layerInfo)) {
            return false;
        }
        
        if (!validateLayerInfo(layerInfo)) {
            return false;
        }
        
        // Add new layer
        canvas.addLayer(layerInfo.name);
        Layer* layer = canvas.getLayer(canvas.getLayerCount() - 1);
        if (!layer) {
            return false;
        }
        
        // Set layer properties
        layer->setOpacity(layerInfo.opacity);
        layer->setVisibility(layerInfo.visible);
        layer->setBlendMode(static_cast<BlendMode>(layerInfo.blendMode));
        
        // Read layer data
        if (!readLayerData(file, layer, layerInfo)) {
            return false;
        }
    }
    
    return true;
}

bool FileSystem::writeProjectHeader(std::ofstream& file, const ProjectHeader& header) {
    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    return file.good();
}

bool FileSystem::readProjectHeader(std::ifstream& file, ProjectHeader& header) {
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    return file.good();
}

bool FileSystem::writeLayerInfo(std::ofstream& file, const Layer* layer) {
    LayerInfo info;
    strncpy(info.name, layer->getName().c_str(), sizeof(info.name) - 1);
    info.width = canvas.getWidth();
    info.height = canvas.getHeight();
    info.opacity = layer->getOpacity();
    info.blendMode = static_cast<uint32_t>(layer->getBlendMode());
    info.visible = layer->getVisibility();
    info.dataOffset = file.tellp();
    
    file.write(reinterpret_cast<const char*>(&info), sizeof(info));
    return file.good();
}

bool FileSystem::readLayerInfo(std::ifstream& file, LayerInfo& info) {
    file.read(reinterpret_cast<char*>(&info), sizeof(info));
    return file.good();
}

bool FileSystem::writeLayerData(std::ofstream& file, const Layer* layer) {
    PixelRect rect{0, 0, canvas.getWidth(), canvas.getHeight()};
    std::vector<float> pixelData = layer->getPixels(rect);
    file.write(reinterpret_cast<const char*>(pixelData.data()),
                     pixelData.size() * sizeof(float));
    return file.good();
}

bool FileSystem::readLayerData(std::ifstream& file, Layer* layer, const LayerInfo& info) {
    std::vector<float> pixelData(info.width * info.height * 4);
    if (!file.read(reinterpret_cast<char*>(pixelData.data()),
                  pixelData.size() * sizeof(float))) {
        return false;
    }
    
    PixelRect rect{0, 0, static_cast<int>(info.width), static_cast<int>(info.height)};
    try {
        layer->setPixels(rect, pixelData);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to set layer pixels: " << e.what() << std::endl;
        return false;
    }
}

bool FileSystem::validateProjectHeader(const ProjectHeader& header) {
    if (header.magic[0] != 'P' || header.magic[1] != 'A' ||
        header.magic[2] != 'I' || header.magic[3] != 'N') {
        return false;
    }

    if (header.version != 1) {
        return false;
    }

    if (header.width == 0 || header.height == 0 ||
        header.width > 16384 || header.height > 16384) {
        return false;
    }
    
    return true;
}

bool FileSystem::validateLayerInfo(const LayerInfo& info) {
    if (info.width == 0 || info.height == 0 ||
        info.width > 16384 || info.height > 16384) {
        return false;
    }
    
    if (info.opacity < 0.0f || info.opacity > 1.0f) {
        return false;
    }

    if (info.blendMode > 3) {
        return false;
    }
    
    return true;
}