#pragma once
#include <string>
#include <memory>
#include <vector>
#include "../core/canvas.hpp"
#include "../core/layer.hpp"

// Used to store project file header information
struct ProjectHeader {
    char magic[4];
    uint32_t version;
    uint32_t width;
    uint32_t height;
    uint32_t numLayers;
    uint32_t flags;
};

// Used to store layer information in project files
struct LayerInfo {
    char name[64];
    uint32_t width;
    uint32_t height;
    float opacity;
    uint32_t blendMode;
    bool visible;
    uint64_t dataOffset; 
    uint64_t dataSize;
};

class FileSystem {
public:
    FileSystem(Canvas& canvas);
    
    // Image file management
    bool importImage(const std::string& path, bool asNewLayer = true);
    bool exportImage(const std::string& path, bool mergeVisible = true);

    // static bool isImageFile(const std::string& path);
    // static std::vector<std::string> getSupportedImageFormats();
    
    // Project file management
    bool saveProject(const std::string& path);
    bool loadProject(const std::string& path);
    
private:
    Canvas& canvas;
    
    // bool loadImageToLayer(const std::string& path, Layer* layer);
    // bool saveLayerToImage(const std::string& path, Layer* layer);
    
    // File I/O functions
    bool writeProjectHeader(std::ofstream& file, const ProjectHeader& header);
    bool readProjectHeader(std::ifstream& file, ProjectHeader& header);
    bool writeLayerInfo(std::ofstream& file, const Layer* layer);
    bool readLayerInfo(std::ifstream& file, LayerInfo& info);
    bool writeLayerData(std::ofstream& file, const Layer* layer);
    bool readLayerData(std::ifstream& file, Layer* layer, const LayerInfo& info);
    
    // Validation functions
    bool validateProjectHeader(const ProjectHeader& header);
    bool validateLayerInfo(const LayerInfo& info);
};