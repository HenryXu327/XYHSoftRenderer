#include "../include/Buffer.h"
#include "../include/MyMath.h"
#include <memory>
#include <algorithm>

// ==================== Buffer 基类 ====================

Buffer::Buffer(unsigned int channel) 
    : channel(channel), 
      width(800), 
      height(600), 
      BUFFER_SIZE(CONF_MAX_BUFFER_WIDTH * CONF_MAX_BUFFER_HEIGHT * channel)
{
    buffer = new unsigned char[BUFFER_SIZE]();
}

Buffer::~Buffer()
{
    delete[] buffer;
    buffer = nullptr;
}

void Buffer::UpdateBufferSize(unsigned int newWidth, unsigned int newHeight)
{
    width = newWidth;
    height = newHeight;
}

// ==================== ColorBuffer 类 ====================

ColorBuffer::ColorBuffer() 
    : Buffer(4) // RGBA 四通道
{
}

ColorBuffer::~ColorBuffer()
{
}

void ColorBuffer::InitWithColor(const COLORREF color)
{
    BYTE r = GetRValue(color);
    BYTE g = GetGValue(color);
    BYTE b = GetBValue(color);
    BYTE a = 255;

    for (unsigned int i = 0; i < height; i++)
    {
        for (unsigned int j = 0; j < width; j++)
        {
            unsigned int index = (i * width + j) * 4;
            if (index + 3 < BUFFER_SIZE)
            {
                buffer[index] = r;     // R
                buffer[index + 1] = g; // G
                buffer[index + 2] = b; // B
                buffer[index + 3] = a; // A
            }
        }
    }
}

void ColorBuffer::InitWithColor(const Vector4f& color)
{
    BYTE r = static_cast<BYTE>(color.x * 255.0f);
    BYTE g = static_cast<BYTE>(color.y * 255.0f);
    BYTE b = static_cast<BYTE>(color.z * 255.0f);
    BYTE a = static_cast<BYTE>(color.w * 255.0f);

    for (unsigned int i = 0; i < height; i++)
    {
        for (unsigned int j = 0; j < width; j++)
        {
            unsigned int index = (i * width + j) * 4;
            if (index + 3 < BUFFER_SIZE)
            {
                buffer[index] = r;     // R
                buffer[index + 1] = g; // G
                buffer[index + 2] = b; // B
                buffer[index + 3] = a; // A
            }
        }
    }
}

// Color版本的方法
void ColorBuffer::InitWithColor(const Color& color)
{
    BYTE r = static_cast<BYTE>(color.r * 255.0f);
    BYTE g = static_cast<BYTE>(color.g * 255.0f);
    BYTE b = static_cast<BYTE>(color.b * 255.0f);
    BYTE a = static_cast<BYTE>(color.a * 255.0f);

    for (unsigned int i = 0; i < height; i++)
    {
        for (unsigned int j = 0; j < width; j++)
        {
            unsigned int index = (i * width + j) * 4;
            if (index + 3 < BUFFER_SIZE)
            {
                buffer[index] = r;     // R
                buffer[index + 1] = g; // G
                buffer[index + 2] = b; // B
                buffer[index + 3] = a; // A
            }
        }
    }
}

// 设置像素颜色（不能设置透明度）
void ColorBuffer::SetPixel(unsigned int x, unsigned int y, COLORREF color)
{
    if (x >= width || y >= height)
        return;

    unsigned int index = (y * width + x) * 4;
    if (index + 3 < BUFFER_SIZE)
    {
        buffer[index] = GetRValue(color);      // R
        buffer[index + 1] = GetGValue(color);  // G
        buffer[index + 2] = GetBValue(color);  // B
        buffer[index + 3] = 255;               // A
    }
}

// 设置像素颜色（可以设置透明度）
void ColorBuffer::SetPixel(unsigned int x, unsigned int y, const Vector4f& color)
{
    if (x >= width || y >= height)
        return;

    unsigned int index = (y * width + x) * 4;
    if (index + 3 < BUFFER_SIZE)
    {
        buffer[index] = static_cast<BYTE>(color.x * 255.0f);     // R
        buffer[index + 1] = static_cast<BYTE>(color.y * 255.0f); // G
        buffer[index + 2] = static_cast<BYTE>(color.z * 255.0f); // B
        buffer[index + 3] = static_cast<BYTE>(color.w * 255.0f); // A
    }
}

// 设置像素颜色（可以设置透明度）
void ColorBuffer::SetPixel(unsigned int x, unsigned int y, const Color& color)
{
    if (x >= width || y >= height)
        return;

    unsigned int index = (y * width + x) * 4;
    if (index + 3 < BUFFER_SIZE)
    {
        buffer[index] = static_cast<BYTE>(color.r * 255.0f);     // R
        buffer[index + 1] = static_cast<BYTE>(color.g * 255.0f); // G
        buffer[index + 2] = static_cast<BYTE>(color.b * 255.0f); // B
        buffer[index + 3] = static_cast<BYTE>(color.a * 255.0f); // A
    }
}

// 获取像素颜色（不能获取透明度）
COLORREF ColorBuffer::GetPixel(unsigned int x, unsigned int y) const
{
    if (x >= width || y >= height)
        return RGB(0, 0, 0);

    unsigned int index = (y * width + x) * 4;
    if (index + 3 < BUFFER_SIZE)
    {
        return RGB(buffer[index], buffer[index + 1], buffer[index + 2]);
    }

    return RGB(0, 0, 0);
}

// 获取像素颜色（可以获取透明度）
Vector4f ColorBuffer::GetPixelVector(unsigned int x, unsigned int y) const
{
    if (x >= width || y >= height)
        return Vector4f(0, 0, 0, 0);

    unsigned int index = (y * width + x) * 4;
    if (index + 3 < BUFFER_SIZE)
    {
        return Vector4f(
            buffer[index] / 255.0f,
            buffer[index + 1] / 255.0f,
            buffer[index + 2] / 255.0f,
            buffer[index + 3] / 255.0f
        );
    }

    return Vector4f(0, 0, 0, 0);
}

// 获取像素颜色（可以获取透明度）
Color ColorBuffer::GetPixelColor(unsigned int x, unsigned int y) const
{
    if (x >= width || y >= height)
        return Color(0, 0, 0, 0);

    unsigned int index = (y * width + x) * 4;
    if (index + 3 < BUFFER_SIZE)
    {
        return Color(
            buffer[index] / 255.0f,
            buffer[index + 1] / 255.0f,
            buffer[index + 2] / 255.0f,
            buffer[index + 3] / 255.0f
        );
    }

    return Color(0, 0, 0, 0);
}

// ==================== DepthBuffer 类 ====================
// 深度缓冲区
DepthBuffer::DepthBuffer()
    : width(800),
      height(600),
      BUFFER_SIZE(CONF_MAX_BUFFER_WIDTH * CONF_MAX_BUFFER_HEIGHT)
{
    buffer = new float[BUFFER_SIZE]();
    // 初始化深度值为1.0（最远）
    InitWithDepth(1.0f);
}

DepthBuffer::~DepthBuffer()
{
    delete[] buffer;
    buffer = nullptr;
}

void DepthBuffer::UpdateBufferSize(unsigned int newWidth, unsigned int newHeight)
{
    width = newWidth;
    height = newHeight;
}

void DepthBuffer::InitWithDepth(float depth)
{
    // 将深度值限制在 [0, 1] 范围内
    depth = clamp01(depth);

    for (unsigned int i = 0; i < height; i++)
    {
        for (unsigned int j = 0; j < width; j++)
        {
            unsigned int index = i * width + j;
            if (index < BUFFER_SIZE)
            {
                buffer[index] = depth;
            }
        }
    }
}

// 设置深度值
void DepthBuffer::SetDepth(unsigned int x, unsigned int y, float depth)
{
    // 将深度值限制在 [0, 1] 范围内
    depth = clamp01(depth);

    if (x >= width || y >= height)
        return;

    unsigned int index = y * width + x;
    if (index < BUFFER_SIZE)
    {
        buffer[index] = depth;
    }
}

// 获取深度值
float DepthBuffer::GetDepth(unsigned int x, unsigned int y) const
{
    if (x >= width || y >= height)
        return 1.0f;

    unsigned int index = y * width + x;
    if (index < BUFFER_SIZE)
    {
        return buffer[index];
    }

    return 1.0f;
}

// ==================== FrameBuffer 类 ====================

FrameBuffer::FrameBuffer()
{
}

FrameBuffer::~FrameBuffer()
{
}

void FrameBuffer::UpdateBufferSize(unsigned int width, unsigned int height)
{
    colorBuffer.UpdateBufferSize(width, height);
    depthBuffer.UpdateBufferSize(width, height);
}

void FrameBuffer::InitWithColorAndDepth(const COLORREF color, float depth)
{
    colorBuffer.InitWithColor(color);
    depthBuffer.InitWithDepth(depth);
}

void FrameBuffer::InitWithColorAndDepth(const Vector4f& color, float depth)
{
    colorBuffer.InitWithColor(color);
    depthBuffer.InitWithDepth(depth);
}

void FrameBuffer::InitWithColorAndDepth(const Color& color, float depth)
{
    colorBuffer.InitWithColor(color);
    depthBuffer.InitWithDepth(depth);
}

// ==================== BufferManager 类 ====================
// 双缓冲区管理（单例类）
BufferManager* BufferManager::s_instance = nullptr;

BufferManager::BufferManager()
    : m_backgroundColor(RGB(0, 0, 0)), 
      m_backgroundColorObj(0.0f, 0.0f, 0.0f, 1.0f)
{
    // 初始化前后缓冲区
    m_frontBuffer.UpdateBufferSize(800, 600);
    m_backBuffer.UpdateBufferSize(800, 600);
    
    m_frontBuffer.InitWithColorAndDepth(m_backgroundColorObj, 1.0f);
    m_backBuffer.InitWithColorAndDepth(m_backgroundColorObj, 1.0f);
}

BufferManager::~BufferManager()
{
}

BufferManager* BufferManager::GetInstance()
{
    if (s_instance == nullptr)
    {
        s_instance = new BufferManager();
    }
    return s_instance;
}

void BufferManager::DeleteInstance()
{
    if (s_instance != nullptr)
    {
        delete s_instance;
        s_instance = nullptr;
    }
}

void BufferManager::UpdateBufferSize(unsigned int width, unsigned int height)
{
    m_frontBuffer.UpdateBufferSize(width, height);
    m_backBuffer.UpdateBufferSize(width, height);
}

void BufferManager::SetBackgroundColor(COLORREF color)
{
    m_backgroundColor = color;
}

void BufferManager::SetBackgroundColor(const Vector4f& color)
{
    BYTE r = static_cast<BYTE>(color.x * 255.0f);
    BYTE g = static_cast<BYTE>(color.y * 255.0f);
    BYTE b = static_cast<BYTE>(color.z * 255.0f);
    m_backgroundColor = RGB(r, g, b);
}

void BufferManager::SetBackgroundColor(const Color& color)
{
    m_backgroundColorObj = color;
    BYTE r = static_cast<BYTE>(color.r * 255.0f);
    BYTE g = static_cast<BYTE>(color.g * 255.0f);
    BYTE b = static_cast<BYTE>(color.b * 255.0f);
    m_backgroundColor = RGB(r, g, b);
}

Color BufferManager::GetBackgroundColor() const
{
    return m_backgroundColorObj;
}

// 获取后缓冲区（用于绘制）
FrameBuffer* BufferManager::GetBackBuffer()
{
    // 清空后缓冲区并返回
    m_backBuffer.InitWithColorAndDepth(m_backgroundColorObj, 1.0f);
    return &m_backBuffer;
}

// 交换前后缓冲区
void BufferManager::SwapBuffers()
{
    // 交换前后缓冲区指针
    // 使用指针交换而不是对象赋值来避免拷贝赋值问题
    std::swap(m_frontBuffer.colorBuffer.buffer, m_backBuffer.colorBuffer.buffer);
    std::swap(m_frontBuffer.depthBuffer.buffer, m_backBuffer.depthBuffer.buffer);
    
    // 确保宽度和高度信息也正确交换
    std::swap(m_frontBuffer.colorBuffer.width, m_backBuffer.colorBuffer.width);
    std::swap(m_frontBuffer.colorBuffer.height, m_backBuffer.colorBuffer.height);
    std::swap(m_frontBuffer.depthBuffer.width, m_backBuffer.depthBuffer.width);
    std::swap(m_frontBuffer.depthBuffer.height, m_backBuffer.depthBuffer.height);
}

// 将当前前缓冲区呈现到设备上下文
void BufferManager::PresentToHDC(HDC hdc)
{
    int width = m_frontBuffer.colorBuffer.width;
    int height = m_frontBuffer.colorBuffer.height;
    
    // 创建一个BITMAPINFO结构，描述我们的位图
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // 负值表示从上到下存储像素
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    // 将颜色缓冲区复制到设备上下文
    SetDIBitsToDevice(
        hdc,                            // 目标设备上下文
        0, 0,                           // 目标左上角位置
        width, height,                  // 位图宽度和高度
        0, 0,                           // 来源左下角位置
        0,                              // 起始扫描线
        height,                         // 扫描线数
        m_frontBuffer.colorBuffer.buffer, // 像素数据
        &bmi,                           // 位图信息
        DIB_RGB_COLORS                  // 使用RGB颜色
    );
} 