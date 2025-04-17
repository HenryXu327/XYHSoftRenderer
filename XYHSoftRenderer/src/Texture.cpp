#include "../include/Texture.h"
#include "../include/MyMath.h"
#include <cstring>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

// 在文件顶部添加此函数的声明
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

// 默认构造函数
Texture::Texture()
    : width(0), height(0), textureData(nullptr), 
    filterMode(TextureFilterMode::BILINEAR), wrapMode(TextureWrapMode::REPEAT),
    hasMipmaps(false)
{
}

// 带尺寸的构造函数
Texture::Texture(int width, int height)
    : width(0), height(0), textureData(nullptr), 
    filterMode(TextureFilterMode::BILINEAR), wrapMode(TextureWrapMode::REPEAT),
    hasMipmaps(false)
{
    Create(width, height);
}

// 拷贝构造函数
Texture::Texture(const Texture& other)
    : width(0), height(0), textureData(nullptr), 
    filterMode(other.filterMode), wrapMode(other.wrapMode),
    hasMipmaps(false)
{
    if (other.width > 0 && other.height > 0) {
        Create(other.width, other.height);
        std::memcpy(textureData, other.textureData, width * height * sizeof(Color));
        
        // 拷贝Mipmap
        if (other.hasMipmaps && other.mipmaps.size() > 0) {
            for (size_t i = 0; i < other.mipmaps.size(); ++i) {
                Texture* mip = new Texture(*other.mipmaps[i]);
                mipmaps.push_back(mip);
            }
            hasMipmaps = true;
        }
    }
}

// 赋值运算符
Texture& Texture::operator=(const Texture& other)
{
    if (this != &other) {
        Clear();
        
        filterMode = other.filterMode;
        wrapMode = other.wrapMode;
        
        if (other.width > 0 && other.height > 0) {
            Create(other.width, other.height);
            std::memcpy(textureData, other.textureData, width * height * sizeof(Color));
            
            // 拷贝Mipmap
            ClearMipmaps();
            if (other.hasMipmaps && other.mipmaps.size() > 0) {
                for (size_t i = 0; i < other.mipmaps.size(); ++i) {
                    Texture* mip = new Texture(*other.mipmaps[i]);
                    mipmaps.push_back(mip);
                }
                hasMipmaps = true;
            }
        }
    }
    return *this;
}

// 析构函数
Texture::~Texture()
{
    ClearMipmaps();
    Clear();
}

// 创建空白纹理
bool Texture::Create(int w, int h)
{
    if (w <= 0 || h <= 0) {
        return false;
    }
    
    Clear();
    
    width = w;
    height = h;
    textureData = new Color[width * height];
    
    // 初始化为黑色
    for (int i = 0; i < width * height; i++) {
        textureData[i] = Color::black;
    }
    
    return true;
}

// 清除Mipmap数据
void Texture::ClearMipmaps()
{
    for (auto mip : mipmaps) {
        delete mip;
    }
    mipmaps.clear();
    hasMipmaps = false;
}

// 清除纹理数据
void Texture::Clear()
{
    ClearMipmaps();
    if (textureData != nullptr) {
        delete[] textureData;
        textureData = nullptr;
    }
    width = 0;
    height = 0;
}

// 从文件加载纹理 - 根据文件扩展名自动选择加载方法
bool Texture::LoadFromFile(const char* path)
{
    std::string pathStr(path);
    std::string extension = pathStr.substr(pathStr.find_last_of('.') + 1);
    
    // 转换为小写
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == "bmp") {
        return LoadFromBMP(path);
    } else if (extension == "jpg" || extension == "jpeg") {
        return LoadFromJPG(path);
    } else {
        std::cerr << "不支持的文件格式: " << extension << std::endl;
        return false;
    }
}

bool Texture::LoadFromFile(const std::string& path)
{
    return LoadFromFile(path.c_str());
}

// 从BMP文件加载纹理
bool Texture::LoadFromBMP(const char* path)
{
    // 打开BMP文件
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << path << std::endl;
        return false;
    }

    // 读取BMP文件头
    const int HEADER_SIZE = 54;
    unsigned char header[HEADER_SIZE];
    file.read(reinterpret_cast<char*>(header), HEADER_SIZE);

    // 检查是否是BMP文件
    if (header[0] != 'B' || header[1] != 'M') {
        std::cerr << "不是有效的BMP文件: " << path << std::endl;
        return false;
    }

    // 解析BMP头信息
    int dataOffset = *reinterpret_cast<int*>(&header[10]);
    int w = *reinterpret_cast<int*>(&header[18]);
    int h = *reinterpret_cast<int*>(&header[22]);
    int bitsPerPixel = *reinterpret_cast<short*>(&header[28]);

    // 目前仅支持24位或32位BMP
    if (bitsPerPixel != 24 && bitsPerPixel != 32) {
        std::cerr << "不支持的BMP格式，必须是24位或32位: " << path << std::endl;
        return false;
    }

    // 创建纹理
    Clear();
    Create(w, h);

    // 计算行字节数（包括填充字节）
    int rowSize = ((w * bitsPerPixel / 8) + 3) & (~3);

    // 跳转到像素数据开始位置
    file.seekg(dataOffset, std::ios::beg);

    // 分配数据缓冲区
    unsigned char* buffer = new unsigned char[rowSize * h];
    file.read(reinterpret_cast<char*>(buffer), rowSize * h);

    // 读取像素数据（注意：BMP存储顺序是从下到上，从左到右，且是BGR格式）
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int bmpY = h - 1 - y; // BMP是从下到上存储的
            int pos = bmpY * rowSize + x * (bitsPerPixel / 8);
            
            // BGR格式
            float b = buffer[pos] / 255.0f;
            float g = buffer[pos + 1] / 255.0f;
            float r = buffer[pos + 2] / 255.0f;
            float a = 1.0f;
            
            // 如果是32位BMP，读取alpha通道
            if (bitsPerPixel == 32) {
                a = buffer[pos + 3] / 255.0f;
            }
            
            // 设置像素
            SetPixel(x, y, Color(r, g, b, a));
        }
    }

    delete[] buffer;
    return true;
}

// 从JPG文件加载纹理
bool Texture::LoadFromJPG(const char* path)
{
    // 初始化GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    // 将char*转换为wchar_t*
    int length = MultiByteToWideChar(CP_ACP, 0, path, -1, NULL, 0);
    wchar_t* wPath = new wchar_t[length];
    MultiByteToWideChar(CP_ACP, 0, path, -1, wPath, length);
    
    // 加载图像
    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(wPath);
    delete[] wPath;
    
    if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok) {
        std::cerr << "无法加载JPG文件: " << path << std::endl;
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return false;
    }
    
    // 获取图像尺寸
    int w = bitmap->GetWidth();
    int h = bitmap->GetHeight();
    
    // 创建纹理
    Clear();
    Create(w, h);
    
    // 读取像素数据
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Gdiplus::Color pixelColor;
            bitmap->GetPixel(x, y, &pixelColor);
            
            // GDI+使用BGRA格式，需要交换红蓝通道
            float b = pixelColor.GetR() / 255.0f;
            float g = pixelColor.GetG() / 255.0f;
            float r = pixelColor.GetB() / 255.0f;
            float a = pixelColor.GetA() / 255.0f;
            
            // 设置像素，交换红蓝通道
            SetPixel(x, y, Color(r, g, b, a));
        }
    }
    
    // 清理
    delete bitmap;
    Gdiplus::GdiplusShutdown(gdiplusToken);
    
    return true;
}

bool Texture::LoadFromJPG(const std::string& path)
{
    return LoadFromJPG(path.c_str());
}

// 获取纹理索引
int Texture::GetIndex(int x, int y) const
{
    // 确保索引在有效范围内
    x = clamp(x, 0, width - 1);
    y = clamp(y, 0, height - 1);
    return y * width + x;
}

// 设置纹理像素
void Texture::SetPixel(int x, int y, const Color& color)
{
    if (textureData && x >= 0 && x < width && y >= 0 && y < height) {
        textureData[GetIndex(x, y)] = color;
    }
}

// 获取纹理像素
Color Texture::GetPixel(int x, int y) const
{
    if (textureData && x >= 0 && x < width && y >= 0 && y < height) {
        return textureData[GetIndex(x, y)];
    }
    return Color::black;
}

// 设置过滤模式
void Texture::SetFilterMode(TextureFilterMode mode)
{
    filterMode = mode;
}

// 设置环绕模式
void Texture::SetWrapMode(TextureWrapMode mode)
{
    wrapMode = mode;
}

// 纹理坐标环绕处理
void Texture::WrapCoordinates(float& u, float& v) const
{
    switch (wrapMode) {
        case TextureWrapMode::REPEAT:
            // 取小数部分，范围[0,1)
            u = u - std::floor(u);
            v = v - std::floor(v);
            break;
            
        case TextureWrapMode::CLAMP:
            // 限制在[0,1]范围内
            u = clamp01(u);
            v = clamp01(v);
            break;
            
        case TextureWrapMode::MIRROR:
            // 对偶数倍使用原始坐标，对奇数倍使用镜像坐标
            u = u - std::floor(u);
            v = v - std::floor(v);
            
            if (static_cast<int>(std::floor(u + 0.5f)) % 2 == 1) {
                u = 1.0f - u;
            }
            if (static_cast<int>(std::floor(v + 0.5f)) % 2 == 1) {
                v = 1.0f - v;
            }
            break;
    }
}

// 最近邻采样
Color Texture::NearestSample(float u, float v) const
{
    WrapCoordinates(u, v);
    
    // 计算像素坐标
    int x = static_cast<int>(u * width);
    int y = static_cast<int>(v * height);
    
    return GetPixel(x, y);
}

// 双线性采样（高耗时）
Color Texture::BilinearSample(float u, float v) const
{
    WrapCoordinates(u, v);
    
    // 计算浮点坐标
    float fx = u * width - 0.5f;
    float fy = v * height - 0.5f;
    
    // 四个相邻像素的整数坐标
    int x0 = static_cast<int>(std::floor(fx));
    int y0 = static_cast<int>(std::floor(fy));
    int x1 = x0 + 1;
    int y1 = y0 + 1;
    
    // 计算权重
    float wx1 = fx - x0;
    float wy1 = fy - y0;
    float wx0 = 1.0f - wx1;
    float wy0 = 1.0f - wy1;
    
    // 获取四个相邻像素的颜色
    Color c00 = GetPixel(x0, y0);
    Color c10 = GetPixel(x1, y0);
    Color c01 = GetPixel(x0, y1);
    Color c11 = GetPixel(x1, y1);
    
    // 双线性插值
    Color result;
    result.r = c00.r * wx0 * wy0 + c10.r * wx1 * wy0 + c01.r * wx0 * wy1 + c11.r * wx1 * wy1;
    result.g = c00.g * wx0 * wy0 + c10.g * wx1 * wy0 + c01.g * wx0 * wy1 + c11.g * wx1 * wy1;
    result.b = c00.b * wx0 * wy0 + c10.b * wx1 * wy0 + c01.b * wx0 * wy1 + c11.b * wx1 * wy1;
    result.a = c00.a * wx0 * wy0 + c10.a * wx1 * wy0 + c01.a * wx0 * wy1 + c11.a * wx1 * wy1;
    
    return result;
}

// 采样纹理（带导数版本，用于Mipmap）
Color Texture::Sample(float u, float v, float dudx, float dvdy) const
{
    if (!textureData || width <= 0 || height <= 0) {
        return Color::black;
    }
    
    // 如果开启了Mipmap且支持三线性过滤
    if (hasMipmaps && filterMode == TextureFilterMode::TRILINEAR && mipmaps.size() > 0) {
        float level = CalculateMipmapLevel(dudx, dvdy);
        return TrilinearSample(u, v, level);
    }
    
    // 否则使用常规采样
    return Sample(u, v);
}

// 计算Mipmap采样级别
float Texture::CalculateMipmapLevel(float dudx, float dvdy) const
{
    // 基于像素导数计算LOD级别
    float dx = dudx * width;
    float dy = dvdy * height;
    
    // 使用最大变化率来选择mipmap级别
    float maxDelta = (std::max)(std::abs(dx), std::abs(dy));
    float level = std::log2(maxDelta);
    
    // 确保级别在有效范围内
    return clamp(level, 0.0f, static_cast<float>(mipmaps.size()));
}

// 三线性采样（在两个mipmap级别之间插值）
Color Texture::TrilinearSample(float u, float v, float level) const
{
    if (level <= 0.0f) {
        // 如果是第0级，直接使用双线性采样
        return BilinearSample(u, v);
    }
    
    // 计算两个相邻的mipmap级别
    int level0 = static_cast<int>(std::floor(level));
    int level1 = level0 + 1;
    
    // 计算两个级别之间的插值因子
    float factor = level - level0;
    
    // 对较小级别进行采样
    Color color0;
    if (level0 == 0) {
        color0 = BilinearSample(u, v);
    } else if (level0 < static_cast<int>(mipmaps.size())) {
        color0 = mipmaps[level0 - 1]->BilinearSample(u, v);
    } else {
        // 使用最小的mipmap级别
        color0 = mipmaps[mipmaps.size() - 1]->BilinearSample(u, v);
    }
    
    // 对较大级别进行采样
    Color color1;
    if (level1 <= static_cast<int>(mipmaps.size())) {
        color1 = mipmaps[level1 - 1]->BilinearSample(u, v);
    } else {
        // 使用最小的mipmap级别
        color1 = mipmaps[mipmaps.size() - 1]->BilinearSample(u, v);
    }
    
    // 线性插值两个颜色
    return Color::lerp(color0, color1, factor);
}

// 生成Mipmap链
void Texture::GenerateMipmaps()
{
    // 清除已有的mipmaps
    ClearMipmaps();
    
    if (!textureData || width <= 1 || height <= 1) {
        return; // 纹理太小，不需要生成mipmap
    }
    
    int currentWidth = width;
    int currentHeight = height;
    Texture* prevMip = this; // 第0级是原始纹理
    
    // 生成mipmap链，直到尺寸为1x1
    while (currentWidth > 1 || currentHeight > 1) {
        int newWidth = (std::max)(1, currentWidth / 2);
        int newHeight = (std::max)(1, currentHeight / 2);
        
        // 创建新的mipmap级别
        Texture* mip = new Texture(newWidth, newHeight);
        mip->filterMode = filterMode;
        mip->wrapMode = wrapMode;
        
        // 对上一级进行2x2区域采样
        for (int y = 0; y < newHeight; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                int x0 = x * 2;
                int y0 = y * 2;
                
                // 获取2x2区域的颜色
                Color c00 = prevMip->GetPixel(x0, y0);
                Color c10 = prevMip->GetPixel(x0 + 1, y0);
                Color c01 = prevMip->GetPixel(x0, y0 + 1);
                Color c11 = prevMip->GetPixel(x0 + 1, y0 + 1);
                
                // 计算平均颜色
                Color avgColor;
                avgColor.r = (c00.r + c10.r + c01.r + c11.r) * 0.25f;
                avgColor.g = (c00.g + c10.g + c01.g + c11.g) * 0.25f;
                avgColor.b = (c00.b + c10.b + c01.b + c11.b) * 0.25f;
                avgColor.a = (c00.a + c10.a + c01.a + c11.a) * 0.25f;
                
                mip->SetPixel(x, y, avgColor);
            }
        }
        
        // 添加到mipmap链
        mipmaps.push_back(mip);
        
        // 更新当前尺寸和上一级mip
        currentWidth = newWidth;
        currentHeight = newHeight;
        prevMip = mip;
    }
    
    hasMipmaps = (mipmaps.size() > 0);
}

// 保存纹理为JPG文件
bool Texture::SaveToJPG(const char* path, int quality) const
{
    if (!textureData || width <= 0 || height <= 0) {
        std::cerr << "Cannot save empty texture to JPG: " << path << std::endl;
        return false;
    }
    
    // 初始化GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    // 创建一个32位ARGB位图
    Gdiplus::Bitmap bitmap(width, height, PixelFormat32bppARGB);
    
    // 写入像素数据
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Color color = GetPixel(x, y);
            Gdiplus::Color gdiColor(
                static_cast<BYTE>(color.a * 255.0f),
                static_cast<BYTE>(color.r * 255.0f),
                static_cast<BYTE>(color.g * 255.0f),
                static_cast<BYTE>(color.b * 255.0f)
            );
            bitmap.SetPixel(x, y, gdiColor);
        }
    }
    
    // 设置JPEG编码参数
    CLSID jpegClsid;
    GetEncoderClsid(L"image/jpeg", &jpegClsid);
    
    Gdiplus::EncoderParameters encoderParams;
    encoderParams.Count = 1;
    encoderParams.Parameter[0].Guid = Gdiplus::EncoderQuality;
    encoderParams.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
    encoderParams.Parameter[0].NumberOfValues = 1;
    ULONG qualityValue = quality;
    encoderParams.Parameter[0].Value = &qualityValue;
    
    // 将路径转换为宽字符
    int length = MultiByteToWideChar(CP_ACP, 0, path, -1, NULL, 0);
    wchar_t* wPath = new wchar_t[length];
    MultiByteToWideChar(CP_ACP, 0, path, -1, wPath, length);
    
    // 保存文件
    Gdiplus::Status status = bitmap.Save(wPath, &jpegClsid, &encoderParams);
    
    // 清理资源
    delete[] wPath;
    Gdiplus::GdiplusShutdown(gdiplusToken);
    
    return (status == Gdiplus::Ok);
}

bool Texture::SaveToJPG(const std::string& path, int quality) const
{
    return SaveToJPG(path.c_str(), quality);
}

// 保存所有Mipmap级别为JPG文件
bool Texture::SaveMipmapsToJPG(const std::string& basePath, int quality) const
{
    if (!textureData || width <= 0 || height <= 0) {
        std::cerr << "Cannot save empty texture's Mipmaps" << std::endl;
        return false;
    }
    
    // 确保basePath不包含扩展名
    std::string basePathWithoutExt = basePath;
    size_t dotPos = basePathWithoutExt.find_last_of('.');
    if (dotPos != std::string::npos) {
        basePathWithoutExt = basePathWithoutExt.substr(0, dotPos);
    }
    
    // 保存基础纹理 (level 0)
    std::string level0Path = basePathWithoutExt + "_mip0.jpg";
    if (!SaveToJPG(level0Path, quality)) {
        return false;
    }
    
    // 保存其他Mipmap级别
    for (size_t i = 0; i < mipmaps.size(); ++i) {
        std::string mipPath = basePathWithoutExt + "_mip" + std::to_string(i + 1) + ".jpg";
        if (!mipmaps[i]->SaveToJPG(mipPath, quality)) {
            return false;
        }
    }
    
    return true;
}

// 获取JPEG编码器的CLSID
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT num = 0;          // 编码器数量
    UINT size = 0;         // 编码器信息大小
    
    // 获取图像编码器数量和大小
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;
    
    // 分配内存
    Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;
    
    // 获取图像编码器信息
    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);
    
    // 查找指定格式的编码器
    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }
    
    free(pImageCodecInfo);
    return -1;
}

// 采样纹理（不带导数版本）
Color Texture::Sample(float u, float v) const
{
    if (!textureData || width <= 0 || height <= 0) {
        return Color::black;
    }
    
    // 根据过滤模式选择采样方法
    switch (filterMode) {
        case TextureFilterMode::NEAREST:
            return NearestSample(u, v);
            
        case TextureFilterMode::BILINEAR:
            return BilinearSample(u, v);
            
        case TextureFilterMode::TRILINEAR:
            if (hasMipmaps) {
                // 如果没有导数信息，则使用最高质量的纹理(Level 0)
                return BilinearSample(u, v);
            } else {
                return BilinearSample(u, v);
            }
            
        default:
            return NearestSample(u, v);
    }
}

// 创建棋盘格纹理
Texture Texture::CreateCheckerboard(int width, int height, int checkSize, const Color& color1, const Color& color2)
{
    Texture texture;
    texture.Create(width, height);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            bool isEvenX = (x / checkSize) % 2 == 0;
            bool isEvenY = (y / checkSize) % 2 == 0;
            
            if (isEvenX ^ isEvenY) {
                texture.SetPixel(x, y, color1);
            } else {
                texture.SetPixel(x, y, color2);
            }
        }
    }
    
    return texture;
}

// 创建渐变纹理
Texture Texture::CreateGradient(int width, int height, const Color& startColor, const Color& endColor, bool horizontal)
{
    Texture texture;
    texture.Create(width, height);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float t;
            if (horizontal) {
                t = static_cast<float>(x) / (width - 1);
            } else {
                t = static_cast<float>(y) / (height - 1);
            }
            
            // 线性插值颜色
            Color color;
            color.r = startColor.r * (1.0f - t) + endColor.r * t;
            color.g = startColor.g * (1.0f - t) + endColor.g * t;
            color.b = startColor.b * (1.0f - t) + endColor.b * t;
            color.a = startColor.a * (1.0f - t) + endColor.a * t;
            
            texture.SetPixel(x, y, color);
        }
    }
    
    return texture;
}

// 创建圆形纹理
Texture Texture::CreateCircle(int size, const Color& circleColor, const Color& backgroundColor)
{
    Texture texture;
    texture.Create(size, size);
    
    float radius = size * 0.5f;
    float centerX = radius;
    float centerY = radius;
    
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            float dx = x - centerX;
            float dy = y - centerY;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance <= radius) {
                texture.SetPixel(x, y, circleColor);
            } else {
                texture.SetPixel(x, y, backgroundColor);
            }
        }
    }
    
    return texture;
}
