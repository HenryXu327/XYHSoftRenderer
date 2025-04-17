#pragma once
#include "Color.h"
#include <string>
#include <vector>

// 纹理过滤模式
enum class TextureFilterMode {
    NEAREST,    // 最近邻过滤
    BILINEAR,   // 双线性过滤
    TRILINEAR   // 三线性过滤（使用Mipmap）
};

// 纹理环绕模式
enum class TextureWrapMode {
    REPEAT,     // 重复
    CLAMP,      // 钳制
    MIRROR      // 镜像
};

class Texture
{
public:
    int width, height;
    Color* textureData;
    TextureFilterMode filterMode;
    TextureWrapMode wrapMode;
    
    // Mipmap相关
    std::vector<Texture*> mipmaps;
    bool hasMipmaps;

    // 构造函数和析构函数
    Texture();
    Texture(int width, int height);
    Texture(const Texture& other); // 拷贝构造函数
    Texture& operator=(const Texture& other); // 赋值运算符
    ~Texture();

    // 创建空白纹理
    bool Create(int width, int height);
    
    // 清除纹理数据
    void Clear();
    
    // 从文件加载纹理
    bool LoadFromFile(const char* path);
    bool LoadFromFile(const std::string& path);
    
    // 从BMP文件加载纹理
    bool LoadFromBMP(const char* path);
    
    // 从JPG文件加载纹理
    bool LoadFromJPG(const char* path);
    bool LoadFromJPG(const std::string& path);
    
    // 保存纹理为JPG文件
    bool SaveToJPG(const char* path, int quality = 90) const;
    bool SaveToJPG(const std::string& path, int quality = 90) const;
    
    // 采样纹理
    Color Sample(float u, float v) const;
    Color Sample(float u, float v, float dudx, float dvdy) const; // 带有导数的采样（用于Mipmap）
    
    // 设置纹理像素
    void SetPixel(int x, int y, const Color& color);
    
    // 获取纹理像素
    Color GetPixel(int x, int y) const;
    
    // 设置过滤模式
    void SetFilterMode(TextureFilterMode mode);
    
    // 设置环绕模式
    void SetWrapMode(TextureWrapMode mode);

    // Mipmap相关函数
    void GenerateMipmaps();
    void ClearMipmaps();
    bool SaveMipmapsToJPG(const std::string& basePath, int quality = 90) const;
    
    // 生成测试纹理（如棋盘格纹理、渐变纹理等）
    static Texture CreateCheckerboard(int width, int height, int checkSize, const Color& color1, const Color& color2);
    static Texture CreateGradient(int width, int height, const Color& startColor, const Color& endColor, bool horizontal = true);
    static Texture CreateCircle(int size, const Color& circleColor, const Color& backgroundColor);

private:
    // 纹理过滤函数
    Color NearestSample(float u, float v) const;
    Color BilinearSample(float u, float v) const;
    Color TrilinearSample(float u, float v, float level) const;
    
    // 纹理环绕函数
    void WrapCoordinates(float& u, float& v) const;
    
    // 计算纹理索引
    int GetIndex(int x, int y) const;

    // 计算Mipmap采样级别
    float CalculateMipmapLevel(float dudx, float dvdy) const;
};