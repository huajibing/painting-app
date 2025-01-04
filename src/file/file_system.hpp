#pragma once
#include <string>
#include <memory>
#include <vector>
#include "../core/canvas.hpp"
#include "../core/layer.hpp"

// 用于存储项目文件头部信息
struct ProjectHeader {
    char magic[4];           // 魔数，用于识别文件类型
    uint32_t version;        // 文件版本号
    uint32_t width;          // 画布宽度
    uint32_t height;         // 画布高度
    uint32_t numLayers;      // 图层数量
    uint32_t flags;          // 额外标志位
};

// 用于存储每个图层的信息
struct LayerInfo {
    char name[64];          // 图层名称
    uint32_t width;         // 图层宽度
    uint32_t height;        // 图层高度
    float opacity;          // 不透明度
    uint32_t blendMode;     // 混合模式
    bool visible;           // 可见性
    uint64_t dataOffset;    // 图层数据在文件中的偏移量
    uint64_t dataSize;      // 图层数据大小
};

class FileSystem {
public:
    FileSystem(Canvas& canvas);
    
    // 图像导入导出功能
    bool importImage(const std::string& path, bool asNewLayer = true);
    bool exportImage(const std::string& path, bool mergeVisible = true);
    
    // 支持的图像格式检查
    static bool isImageFile(const std::string& path);
    static std::vector<std::string> getSupportedImageFormats();
    
    // 项目文件管理
    bool saveProject(const std::string& path);
    bool loadProject(const std::string& path);
    
private:
    Canvas& canvas;
    
    // 图像处理辅助函数
    bool loadImageToLayer(const std::string& path, Layer* layer);
    bool saveLayerToImage(const std::string& path, Layer* layer);
    
    // 项目文件辅助函数
    bool writeProjectHeader(std::ofstream& file, const ProjectHeader& header);
    bool readProjectHeader(std::ifstream& file, ProjectHeader& header);
    bool writeLayerInfo(std::ofstream& file, const Layer* layer);
    bool readLayerInfo(std::ifstream& file, LayerInfo& info);
    bool writeLayerData(std::ofstream& file, const Layer* layer);
    bool readLayerData(std::ifstream& file, Layer* layer, const LayerInfo& info);
    
    // 验证函数
    bool validateProjectHeader(const ProjectHeader& header);
    bool validateLayerInfo(const LayerInfo& info);
};