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
    
    // 创建新图层或使用当前图层
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
    
    // 转换图像数据到浮点格式
    std::vector<float> pixelData;
    pixelData.reserve(width * height * 4);
    
    for (int i = 0; i < width * height * 4; ++i) {
        pixelData.push_back(data[i] / 255.0f);
    }
    
    stbi_image_free(data);
    
    // 设置图层数据
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
    // 获取需要导出的图层
    Layer* layer = nullptr;
    if (mergeVisible) {
        // TODO: 实现图层合并功能
        // layer = canvas.mergeLayers();
    } else {
        layer = canvas.getLayer(canvas.getActiveLayerIndex());
    }
    
    if (!layer) {
        return false;
    }
    
    // 获取图层数据
    PixelRect rect{0, 0, canvas.getWidth(), canvas.getHeight()};
    std::vector<float> pixelData = layer->getPixels(rect);
    
    // 转换为8位格式
    std::vector<unsigned char> outputData;
    outputData.reserve(pixelData.size());
    
    for (float value : pixelData) {
        outputData.push_back(static_cast<unsigned char>(value * 255.0f));
    }
    
    // 保存图像
    std::string extension = path.substr(path.find_last_of(".") + 1);
    bool success = false;
    
    if (extension == "png") {
        success = stbi_write_png(path.c_str(), canvas.getWidth(), canvas.getHeight(),
                               4, outputData.data(), canvas.getWidth() * 4);
    } else if (extension == "jpg" || extension == "jpeg") {
        success = stbi_write_jpg(path.c_str(), canvas.getWidth(), canvas.getHeight(),
                               4, outputData.data(), 95);
    }
    
    return success;
}

bool FileSystem::saveProject(const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    
    // 写入文件头
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
    
    // 写入每个图层的信息和数据
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
    
    // 读取文件头
    ProjectHeader header;
    if (!readProjectHeader(file, header)) {
        return false;
    }
    
    if (!validateProjectHeader(header)) {
        return false;
    }
    
    // 清除现有图层
    // TODO: 实现canvas.clear()
    
    // 读取每个图层
    for (uint32_t i = 0; i < header.numLayers; ++i) {
        LayerInfo layerInfo;
        if (!readLayerInfo(file, layerInfo)) {
            return false;
        }
        
        if (!validateLayerInfo(layerInfo)) {
            return false;
        }
        
        // 创建新图层
        canvas.addLayer(layerInfo.name);
        Layer* layer = canvas.getLayer(canvas.getLayerCount() - 1);
        if (!layer) {
            return false;
        }
        
        // 设置图层属性
        layer->setOpacity(layerInfo.opacity);
        layer->setVisibility(layerInfo.visible);
        layer->setBlendMode(static_cast<BlendMode>(layerInfo.blendMode));
        
        // 读取图层数据
        if (!readLayerData(file, layer, layerInfo)) {
            return false;
        }
    }
    
    return true;
}

// 项目文件辅助函数实现
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
    info.dataOffset = file.tellp();  // 当前文件位置作为数据偏移量
    
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
    // 检查魔数
    if (header.magic[0] != 'P' || header.magic[1] != 'A' ||
        header.magic[2] != 'I' || header.magic[3] != 'N') {
        return false;
    }
    
    // 检查版本号
    if (header.version != 1) {
        return false;
    }
    
    // 检查画布尺寸
    if (header.width == 0 || header.height == 0 ||
        header.width > 16384 || header.height > 16384) {
        return false;
    }
    
    return true;
}

bool FileSystem::validateLayerInfo(const LayerInfo& info) {
    // 检查图层尺寸
    if (info.width == 0 || info.height == 0 ||
        info.width > 16384 || info.height > 16384) {
        return false;
    }
    
    // 检查不透明度范围
    if (info.opacity < 0.0f || info.opacity > 1.0f) {
        return false;
    }
    
    // 检查混合模式
    if (info.blendMode > 3) {  // 假设只有4种混合模式
        return false;
    }
    
    return true;
}