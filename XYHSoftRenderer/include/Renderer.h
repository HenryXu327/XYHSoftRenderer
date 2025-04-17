#pragma once

#include <Windows.h>
#include <string>
#include <memory>
#include "Buffer.h"
#include "Color.h"
#include "Object.h"
#include "Shader.h"

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    bool Initialize(HDC hdc);
    void Shutdown();

    // 绘制功能
    void SetPixel(int x, int y, COLORREF color);
    void SetPixel(int x, int y, const Color& color);
    void DrawLine(int x1, int y1, int x2, int y2, COLORREF color);
    void DrawLine(int x1, int y1, int x2, int y2, const Color& color);
    void DrawText(int x, int y, const std::wstring& text, COLORREF color);
    void DrawText(int x, int y, const std::wstring& text, const Color& color);
    
    // 3D绘制功能
    void DrawTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, Shader* shader);
    void DrawMesh(const Mesh& mesh, const Matrix& modelMatrix, Shader* shader);
    void DrawObject(const Object& object, Shader* shader);
    
    // 矩阵设置
    void SetModelMatrix(const Matrix& matrix) { m_modelMatrix = matrix; }
    void SetViewMatrix(const Matrix& matrix) { m_viewMatrix = matrix; }
    void SetProjectionMatrix(const Matrix& matrix) { m_projMatrix = matrix; }
    
    // 相机位置设置
    void SetViewPosition(const Vector3f& position) { m_viewPosition = position; }
    Vector3f GetViewPosition() const { return m_viewPosition; }
    
    // 矩阵获取
    Matrix GetModelMatrix() const { return m_modelMatrix; }
    Matrix GetViewMatrix() const { return m_viewMatrix; }
    Matrix GetProjectionMatrix() const { return m_projMatrix; }
    
    // 剔除模式
    enum class CullMode {
        CULL_BACK,      // 剔除背面（默认）
        CULL_FRONT,     // 剔除正面 
        CULL_NONE       // 双面绘制
    };
    void SetCullMode(CullMode mode) { m_cullMode = mode; }
    CullMode GetCullMode() const { return m_cullMode; }
    
    // 设置面的绘制顺序
    enum class FrontFace {
        COUNTER_CLOCKWISE,  // 逆时针为正面（默认）
        CLOCKWISE           // 顺时针为正面
    };
    void SetFrontFace(FrontFace order) { m_frontFace = order; }
    FrontFace GetFrontFace() const { return m_frontFace; }
    
    // 缓冲区操作
    void ClearBackBuffer(COLORREF color);
    void ClearBackBuffer(const Color& color);
    void ClearDepthBuffer(float depth = 1.0f);  // 清空深度缓冲区
    void SwapBuffers(HDC hdc);

    // 获取尺寸
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    
    // 设置背景颜色
    void SetBackgroundColor(COLORREF color);
    void SetBackgroundColor(const Color& color);

    // 获取背景颜色
    Color GetBackgroundColor() const;

private:
    // 处理顶点着色器输出，进行透视除法和视口变换
    void ProcessVertexOutput(VertexOutput& vertex);
    
    // 判断点是否在三角形内，并返回重心坐标
    bool PointInTriangle(float x, float y, const Vector2f& v1, const Vector2f& v2, const Vector2f& v3, float& w1, float& w2, float& w3);
    
    // 插值顶点数据
    VertexOutput InterpolateVertex(const VertexOutput& v1, const VertexOutput& v2, const VertexOutput& v3, float w1, float w2, float w3);
    
    // 对三角形进行近平面裁剪，返回0-2个新的三角形
    std::vector<std::array<VertexOutput, 3>> ClipTriangleAgainstNearPlane(const VertexOutput& v1, const VertexOutput& v2, const VertexOutput& v3);
    
    // 线段与近平面求交点
    VertexOutput ClipAgainstNearPlane(const VertexOutput& v1, const VertexOutput& v2);

private:
    // 尺寸
    int m_width; // 窗口宽度
    int m_height; // 窗口高度
    
    // 变换矩阵
    Matrix m_modelMatrix;
    Matrix m_viewMatrix;
    Matrix m_projMatrix;
    
    // 相机位置
    Vector3f m_viewPosition;
    
    // 绘制状态
    CullMode m_cullMode;    // 剔除模式
    FrontFace m_frontFace;  // 面的朝向判定
    
    // GDI绘图相关（仅用于文本绘制）
    HDC m_memDC;           // 内存DC
    HBITMAP m_hOldBitmap;  // 旧位图句柄
    
    // 缓冲区管理器
    BufferManager* m_bufferManager;
    
    // 当前绘制帧缓冲区
    FrameBuffer* m_currentFrameBuffer;
}; 