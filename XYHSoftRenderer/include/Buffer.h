#pragma once

#include <Windows.h>
#include <cassert>
#include "Vector.h"
#include "Color.h"

// 缓冲区尺寸配置
#define CONF_MAX_BUFFER_WIDTH 1920
#define CONF_MAX_BUFFER_HEIGHT 1080

// 基础缓冲区类
class Buffer {
protected:
    const DWORD BUFFER_SIZE;

public:
    unsigned int channel;    // 通道数
    unsigned int width;      // 宽度
    unsigned int height;     // 高度
    unsigned char* buffer;   // 缓冲区数据

    Buffer(unsigned int channel);
    virtual ~Buffer();

    // 更新缓冲区大小
    void UpdateBufferSize(unsigned int width, unsigned int height);
};

// RGBA颜色缓冲区
class ColorBuffer : public Buffer {
public:
    ColorBuffer();
    ~ColorBuffer();

    // 用指定颜色初始化缓冲区
    void InitWithColor(const COLORREF color);
    void InitWithColor(const Vector4f& color);
    void InitWithColor(const Color& color);

    // 设置像素颜色
    void SetPixel(unsigned int x, unsigned int y, COLORREF color);
    void SetPixel(unsigned int x, unsigned int y, const Vector4f& color);
    void SetPixel(unsigned int x, unsigned int y, const Color& color);

    // 获取像素颜色
    COLORREF GetPixel(unsigned int x, unsigned int y) const;
    Vector4f GetPixelVector(unsigned int x, unsigned int y) const;
    Color GetPixelColor(unsigned int x, unsigned int y) const;

    // 获取缓冲区数据指针
    COLORREF* GetBuffer() const { return reinterpret_cast<COLORREF*>(buffer); }
};

// 深度缓冲区
class DepthBuffer {
protected:
    const DWORD BUFFER_SIZE;

public:
    unsigned int width;      // 宽度
    unsigned int height;     // 高度
    float* buffer;           // 浮点深度值缓冲区

    DepthBuffer();
    ~DepthBuffer();

    // 更新缓冲区大小
    void UpdateBufferSize(unsigned int width, unsigned int height);

    // 用指定深度值初始化缓冲区
    void InitWithDepth(float depth);

    // 设置深度值
    void SetDepth(unsigned int x, unsigned int y, float depth);

    // 获取深度值
    float GetDepth(unsigned int x, unsigned int y) const;
};

// 帧缓冲区（包含颜色和深度缓冲区）
class FrameBuffer {
public:
    ColorBuffer colorBuffer;
    DepthBuffer depthBuffer;

    FrameBuffer();
    ~FrameBuffer();

    // 更新缓冲区大小
    void UpdateBufferSize(unsigned int width, unsigned int height);

    // 用指定颜色和深度初始化缓冲区
    void InitWithColorAndDepth(const COLORREF color, float depth);
    void InitWithColorAndDepth(const Vector4f& color, float depth);
    void InitWithColorAndDepth(const Color& color, float depth);
};

// 双缓冲区管理
class BufferManager {
private:
    static BufferManager* s_instance;  // 单例实例

    FrameBuffer m_frontBuffer;         // 前置缓冲区
    FrameBuffer m_backBuffer;          // 后置缓冲区
    COLORREF m_backgroundColor;        // 背景颜色
    Color m_backgroundColorObj;        // 背景颜色对象
    
    // 构造和析构函数
    BufferManager();
    ~BufferManager();

public:
    // 获取单例实例
    static BufferManager* GetInstance();
    static void DeleteInstance();

    // 更新缓冲区大小
    void UpdateBufferSize(unsigned int width, unsigned int height);

    // 设置背景颜色
    void SetBackgroundColor(COLORREF color);
    void SetBackgroundColor(const Vector4f& color);
    void SetBackgroundColor(const Color& color);

    // 获取背景颜色
    Color GetBackgroundColor() const;

    // 获取后置缓冲区（用于绘制）
    FrameBuffer* GetBackBuffer();

    // 交换前后缓冲区
    void SwapBuffers();

    // 将当前前置缓冲区加载到GDI设备
    void PresentToHDC(HDC hdc);
}; 