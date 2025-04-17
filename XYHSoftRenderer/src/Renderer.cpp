#include "../include/Renderer.h"
#include <memory>
#include <algorithm>
#include <vector>
#include <array>

Renderer::Renderer(int width, int height)
    : m_width(width), m_height(height),
    m_memDC(nullptr), m_hOldBitmap(nullptr),
    m_bufferManager(nullptr), m_currentFrameBuffer(nullptr),
    m_modelMatrix(Matrix::identity()),
    m_viewMatrix(Matrix::identity()),
    m_projMatrix(Matrix::identity()),
    m_viewPosition(0.0f, 0.0f, 10.0f),
    m_cullMode(CullMode::CULL_BACK),
    m_frontFace(FrontFace::COUNTER_CLOCKWISE)
{
}

Renderer::~Renderer()
{
    Shutdown();
}

bool Renderer::Initialize(HDC hdc)
{
    // 获取缓冲区管理器实例
    m_bufferManager = BufferManager::GetInstance();
    m_bufferManager->UpdateBufferSize(m_width, m_height);
    
    // 初始化GDI资源（用于文本绘制）
    m_memDC = CreateCompatibleDC(hdc);
    if (!m_memDC)
        return false;
    
    // 获取绘制用的帧缓冲区
    m_currentFrameBuffer = m_bufferManager->GetBackBuffer();
    
    // 清空缓冲区为黑色
    ClearBackBuffer(Color::black);

    return true;
}

void Renderer::Shutdown()
{
    if (m_memDC)
    {
        DeleteDC(m_memDC);
        m_memDC = nullptr;
    }
    
    // 清理缓冲区管理器
    if (m_bufferManager)
    {
        BufferManager::DeleteInstance();
        m_bufferManager = nullptr;
    }
    
    m_currentFrameBuffer = nullptr;
}

void Renderer::SetPixel(int x, int y, COLORREF color)
{
    if (x < 0 || x >= m_width || y < 0 || y >= m_height)
        return;
    
    m_currentFrameBuffer->colorBuffer.SetPixel(x, y, color);
}

void Renderer::SetPixel(int x, int y, const Color& color)
{
    if (x < 0 || x >= m_width || y < 0 || y >= m_height)
        return;
    
    m_currentFrameBuffer->colorBuffer.SetPixel(x, y, color);
}

void Renderer::DrawLine(int x1, int y1, int x2, int y2, COLORREF color)
{
    // 将COLORREF转换为Color
    BYTE r = GetRValue(color);
    BYTE g = GetGValue(color);
    BYTE b = GetBValue(color);
    Color colorObj(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    
    // 调用Color版本
    DrawLine(x1, y1, x2, y2, colorObj);
}

void Renderer::DrawLine(int x1, int y1, int x2, int y2, const Color& color)
{
    // 数字微分分析器(DDA)算法实现
    int dx = x2 - x1;
    int dy = y2 - y1;
    
    // 计算步进数
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
    
    // 计算每一步的增量
    float xIncrement = static_cast<float>(dx) / steps;
    float yIncrement = static_cast<float>(dy) / steps;
    
    // 初始位置
    float x = static_cast<float>(x1);
    float y = static_cast<float>(y1);
    
    // 绘制第一个点
    SetPixel(round(x), round(y), color);
    
    // 绘制剩余的点
    for (int i = 0; i < steps; i++)
    {
        x += xIncrement;
        y += yIncrement;
        SetPixel(round(x), round(y), color);
    }
}

void Renderer::DrawText(int x, int y, const std::wstring& text, COLORREF color)
{
    // 将COLORREF转换为Color
    BYTE r = GetRValue(color);
    BYTE g = GetGValue(color);
    BYTE b = GetBValue(color);
    Color colorObj(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    
    // 调用Color版本
    DrawText(x, y, text, colorObj);
}

void Renderer::DrawText(int x, int y, const std::wstring& text, const Color& color)
{
    // 创建一个与当前屏幕兼容的DC
    HDC hdc = CreateCompatibleDC(m_memDC);
    if (!hdc) 
        return;
    
    // 创建一个位图用于文本绘制
    HBITMAP hBitmap = CreateCompatibleBitmap(m_memDC, m_width, m_height);
    if (!hBitmap) {
        DeleteDC(hdc);
        return;
    }
    
    // 选择位图到DC
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdc, hBitmap);
    
    // 使用黑色填充背景
    RECT rc = { 0, 0, m_width, m_height };
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);
    
    // 设置文本颜色和背景模式
    COLORREF colorRef = RGB(
        static_cast<BYTE>(color.r * 255.0f),
        static_cast<BYTE>(color.g * 255.0f),
        static_cast<BYTE>(color.b * 255.0f)
    );
    SetTextColor(hdc, colorRef);
    SetBkMode(hdc, TRANSPARENT);
    
    // 绘制文本
    TextOutW(hdc, 0, 0, text.c_str(), (int)text.length());
    
    // 获取位图数据
    BITMAP bm;
    GetObject(hBitmap, sizeof(BITMAP), &bm);
    
    // 计算需要的文本区域
    SIZE size;
    GetTextExtentPoint32W(hdc, text.c_str(), (int)text.length(), &size);
    
    // 获取位图数据
    int textWidth = size.cx;
    int textHeight = size.cy;
    
    // 创建BITMAPINFO结构
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = textWidth;
    bmi.bmiHeader.biHeight = -textHeight; // 负值表示从上到下
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    // 分配内存
    std::vector<BYTE> bits(textWidth * textHeight * 4);
    
    // 获取位图数据
    GetDIBits(hdc, hBitmap, 0, textHeight, bits.data(), &bmi, DIB_RGB_COLORS);
    
    // 将非黑色像素复制到缓冲区
    for (int i = 0; i < textHeight; i++) {
        for (int j = 0; j < textWidth; j++) {
            int index = (i * textWidth + j) * 4;
            if (index + 2 < bits.size()) {
                BYTE b = bits[index];
                BYTE g = bits[index + 1];
                BYTE r = bits[index + 2];
                
                // 检查像素是否非黑色
                if (r > 0 || g > 0 || b > 0) {
                    // 将像素设置到颜色缓冲区
                    int pixelX = x + j;
                    int pixelY = y + i;
                    
                    if (pixelX >= 0 && pixelX < m_width && pixelY >= 0 && pixelY < m_height) {
                        // 使用原始颜色的透明度创建新颜色
                        Color textColor(r / 255.0f, g / 255.0f, b / 255.0f, color.a);
                        m_currentFrameBuffer->colorBuffer.SetPixel(pixelX, pixelY, textColor);
                    }
                }
            }
        }
    }
    
    // 清理资源
    SelectObject(hdc, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hdc);
}

void Renderer::ClearBackBuffer(COLORREF color)
{
    // 将COLORREF转换为Color
    BYTE r = GetRValue(color);
    BYTE g = GetGValue(color);
    BYTE b = GetBValue(color);
    Color colorObj(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    
    // 调用Color版本
    ClearBackBuffer(colorObj);
}

void Renderer::ClearBackBuffer(const Color& color)
{
    m_currentFrameBuffer->InitWithColorAndDepth(color, 1.0f);
}

void Renderer::ClearDepthBuffer(float depth)
{
    m_currentFrameBuffer->depthBuffer.InitWithDepth(depth);
}

void Renderer::SwapBuffers(HDC hdc)
{
    // 交换缓冲区
    m_bufferManager->SwapBuffers();
    
    // 将前缓冲区呈现到目标设备上下文
    m_bufferManager->PresentToHDC(hdc);
    
    // 获取新的后缓冲区用于下一帧绘制
    m_currentFrameBuffer = m_bufferManager->GetBackBuffer();
}

void Renderer::SetBackgroundColor(COLORREF color)
{
    // 将COLORREF转换为Color
    BYTE r = GetRValue(color);
    BYTE g = GetGValue(color);
    BYTE b = GetBValue(color);
    Color colorObj(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    
    // 调用Color版本
    SetBackgroundColor(colorObj);
}

void Renderer::SetBackgroundColor(const Color& color)
{
    m_bufferManager->SetBackgroundColor(color);
}

Color Renderer::GetBackgroundColor() const
{
    return m_bufferManager->GetBackgroundColor();
}

// 处理顶点着色器输出，进行透视除法和视口变换
void Renderer::ProcessVertexOutput(VertexOutput& vertex)
{
    // 透视除法
    float w = vertex.position.w;
    if (std::abs(w) < 0.001f) {
        w = 0.001f;  // 避免除以零
    }
    
    vertex.position.x /= w;
    vertex.position.y /= w;
    vertex.position.z /= w;
    
    // 视口变换
    vertex.position.x = (vertex.position.x + 1.0f) * m_width * 0.5f;
    vertex.position.y = (1.0f - vertex.position.y) * m_height * 0.5f;  // Y轴翻转
    vertex.position.z = vertex.position.z * 0.5f + 0.5f;  // 将z从[-1,1]映射到[0,1]
}

// 计算点到线的距离，用于判断点是否在三角形内
inline float EdgeFunction(const Vector2f& a, const Vector2f& b, const Vector2f& c) 
{
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

// 判断点是否在三角形内，并返回重心坐标
bool Renderer::PointInTriangle(float x, float y, const Vector2f& v1, const Vector2f& v2, const Vector2f& v3, float& w1, float& w2, float& w3) 
{
    // 计算三角形面积的两倍（只计算一次）
    const float area = EdgeFunction(v1, v2, v3);
    
    // 检查面积是否为零
    if (std::abs(area) < 0.00001f) {
        return false;
    }
    
    // 计算重心坐标
    const float invArea = 1.0f / area;
    w1 = EdgeFunction(v2, v3, Vector2f(x, y)) * invArea;
    
    // 提前检查第一个坐标，避免不必要的计算
    if (w1 < 0.0f) return false;
    
    w2 = EdgeFunction(v3, v1, Vector2f(x, y)) * invArea;
    
    // 提前检查第二个坐标
    if (w2 < 0.0f) return false;
    
    w3 = 1.0f - w1 - w2; // 避免第三次EdgeFunction调用
    
    // 只需检查第三个坐标
    return w3 >= 0.0f;
}

// 插值顶点数据
VertexOutput Renderer::InterpolateVertex(const VertexOutput& v1, const VertexOutput& v2, const VertexOutput& v3, float w1, float w2, float w3)
{
    VertexOutput result;
    
    // 进行透视校正插值
    float z1 = v1.position.z;
    float z2 = v2.position.z;
    float z3 = v3.position.z;
    
    // 计算插值的z值
    result.position.z = w1 * z1 + w2 * z2 + w3 * z3;
    
    // 透视校正因子
    float correctionW1 = w1 / v1.position.w;
    float correctionW2 = w2 / v2.position.w;
    float correctionW3 = w3 / v3.position.w;
    
    // 归一化因子
    float normalizer = 1.0f / (correctionW1 + correctionW2 + correctionW3);
    
    // 透视校正插值颜色
    result.color.x = (correctionW1 * v1.color.x + correctionW2 * v2.color.x + correctionW3 * v3.color.x) * normalizer;
    result.color.y = (correctionW1 * v1.color.y + correctionW2 * v2.color.y + correctionW3 * v3.color.y) * normalizer;
    result.color.z = (correctionW1 * v1.color.z + correctionW2 * v2.color.z + correctionW3 * v3.color.z) * normalizer;
    result.color.w = (correctionW1 * v1.color.w + correctionW2 * v2.color.w + correctionW3 * v3.color.w) * normalizer;
    
    // 透视校正插值法线
    result.normal.x = (correctionW1 * v1.normal.x + correctionW2 * v2.normal.x + correctionW3 * v3.normal.x) * normalizer;
    result.normal.y = (correctionW1 * v1.normal.y + correctionW2 * v2.normal.y + correctionW3 * v3.normal.y) * normalizer;
    result.normal.z = (correctionW1 * v1.normal.z + correctionW2 * v2.normal.z + correctionW3 * v3.normal.z) * normalizer;
    result.normal = result.normal.normalize();  // 重新归一化
    
    // 透视校正插值纹理坐标
    result.texcoord.x = (correctionW1 * v1.texcoord.x + correctionW2 * v2.texcoord.x + correctionW3 * v3.texcoord.x) * normalizer;
    result.texcoord.y = (correctionW1 * v1.texcoord.y + correctionW2 * v2.texcoord.y + correctionW3 * v3.texcoord.y) * normalizer;
    
    // 透视校正插值世界坐标
    result.worldPos.x = (correctionW1 * v1.worldPos.x + correctionW2 * v2.worldPos.x + correctionW3 * v3.worldPos.x) * normalizer;
    result.worldPos.y = (correctionW1 * v1.worldPos.y + correctionW2 * v2.worldPos.y + correctionW3 * v3.worldPos.y) * normalizer;
    result.worldPos.z = (correctionW1 * v1.worldPos.z + correctionW2 * v2.worldPos.z + correctionW3 * v3.worldPos.z) * normalizer;
    
    // 屏幕坐标
    result.position.x = w1 * v1.position.x + w2 * v2.position.x + w3 * v3.position.x;
    result.position.y = w1 * v1.position.y + w2 * v2.position.y + w3 * v3.position.y;
    result.position.w = 1.0f;  // 已完成透视除法
    
    return result;
}

// 线段与近平面求交点，通过插值计算新顶点
VertexOutput Renderer::ClipAgainstNearPlane(const VertexOutput& v1, const VertexOutput& v2)
{
    // 近平面对应的W值，应与相机设置中的近平面值保持一致
    // 在投影矩阵应用后，近平面会被映射到w=nearZ的位置
    const float NEAR_PLANE = 0.1f;
    
    // 计算裁剪系数t，使得v1 + t*(v2-v1)位于近平面上
    float t = (NEAR_PLANE - v1.position.w) / (v2.position.w - v1.position.w);
    
    // 插值计算新顶点
    VertexOutput result;
    
    // 裁剪空间位置插值
    result.position.x = v1.position.x + t * (v2.position.x - v1.position.x);
    result.position.y = v1.position.y + t * (v2.position.y - v1.position.y);
    result.position.z = v1.position.z + t * (v2.position.z - v1.position.z);
    result.position.w = NEAR_PLANE; // 交点正好在近平面上
    
    // 世界坐标插值
    result.worldPos.x = v1.worldPos.x + t * (v2.worldPos.x - v1.worldPos.x);
    result.worldPos.y = v1.worldPos.y + t * (v2.worldPos.y - v1.worldPos.y);
    result.worldPos.z = v1.worldPos.z + t * (v2.worldPos.z - v1.worldPos.z);
    
    // 纹理坐标插值
    result.texcoord.x = v1.texcoord.x + t * (v2.texcoord.x - v1.texcoord.x);
    result.texcoord.y = v1.texcoord.y + t * (v2.texcoord.y - v1.texcoord.y);
    
    // 法线插值
    result.normal.x = v1.normal.x + t * (v2.normal.x - v1.normal.x);
    result.normal.y = v1.normal.y + t * (v2.normal.y - v1.normal.y);
    result.normal.z = v1.normal.z + t * (v2.normal.z - v1.normal.z);
    result.normal = result.normal.normalize(); // 重新归一化
    
    // 颜色插值
    result.color.x = v1.color.x + t * (v2.color.x - v1.color.x);
    result.color.y = v1.color.y + t * (v2.color.y - v1.color.y);
    result.color.z = v1.color.z + t * (v2.color.z - v1.color.z);
    result.color.w = v1.color.w + t * (v2.color.w - v1.color.w);
    
    return result;
}

// 对三角形进行近平面裁剪，返回0-2个新的三角形
std::vector<std::array<VertexOutput, 3>> Renderer::ClipTriangleAgainstNearPlane(
    const VertexOutput& v1, const VertexOutput& v2, const VertexOutput& v3)
{
    // 存储裁剪后的三角形
    std::vector<std::array<VertexOutput, 3>> result;
    
    // 近平面对应的W值，应与相机设置中的近平面值保持一致
    const float NEAR_PLANE = 0.1f;
    
    // 判断每个顶点是否在近平面后面(w >= NEAR_PLANE)
    bool v1Inside = v1.position.w >= NEAR_PLANE;
    bool v2Inside = v2.position.w >= NEAR_PLANE;
    bool v3Inside = v3.position.w >= NEAR_PLANE;
    
    // 统计在近平面后面的顶点数量
    int insideCount = (v1Inside ? 1 : 0) + (v2Inside ? 1 : 0) + (v3Inside ? 1 : 0);
    
    // 根据不同情况处理
    if (insideCount == 0) {
        // 所有顶点都在近平面前面，整个三角形被裁剪掉
        return result; // 返回空结果
    }
    else if (insideCount == 3) {
        // 所有顶点都在近平面后面，保留整个三角形
        std::array<VertexOutput, 3> triangle = {v1, v2, v3};
        result.push_back(triangle);
        return result;
    }
    else if (insideCount == 1) {
        // 只有一个顶点在近平面后面，形成一个新三角形
        if (v1Inside) {
            VertexOutput newV2 = ClipAgainstNearPlane(v1, v2);
            VertexOutput newV3 = ClipAgainstNearPlane(v1, v3);
            std::array<VertexOutput, 3> triangle = {v1, newV2, newV3};
            result.push_back(triangle);
        }
        else if (v2Inside) {
            VertexOutput newV1 = ClipAgainstNearPlane(v2, v1);
            VertexOutput newV3 = ClipAgainstNearPlane(v2, v3);
            std::array<VertexOutput, 3> triangle = {newV1, v2, newV3};
            result.push_back(triangle);
        }
        else { // v3Inside
            VertexOutput newV1 = ClipAgainstNearPlane(v3, v1);
            VertexOutput newV2 = ClipAgainstNearPlane(v3, v2);
            std::array<VertexOutput, 3> triangle = {newV1, newV2, v3};
            result.push_back(triangle);
        }
    }
    else { // insideCount == 2
        // 有两个顶点在近平面后面，形成两个新三角形
        if (!v1Inside) {
            VertexOutput newV1A = ClipAgainstNearPlane(v2, v1);
            VertexOutput newV1B = ClipAgainstNearPlane(v3, v1);
            std::array<VertexOutput, 3> triangle1 = {newV1A, v2, v3};
            std::array<VertexOutput, 3> triangle2 = {newV1A, v3, newV1B};
            result.push_back(triangle1);
            result.push_back(triangle2);
        }
        else if (!v2Inside) {
            VertexOutput newV2A = ClipAgainstNearPlane(v1, v2);
            VertexOutput newV2B = ClipAgainstNearPlane(v3, v2);
            std::array<VertexOutput, 3> triangle1 = {v1, newV2A, v3};
            std::array<VertexOutput, 3> triangle2 = {newV2A, newV2B, v3};
            result.push_back(triangle1);
            result.push_back(triangle2);
        }
        else { // !v3Inside
            VertexOutput newV3A = ClipAgainstNearPlane(v1, v3);
            VertexOutput newV3B = ClipAgainstNearPlane(v2, v3);
            std::array<VertexOutput, 3> triangle1 = {v1, v2, newV3A};
            std::array<VertexOutput, 3> triangle2 = {v2, newV3B, newV3A};
            result.push_back(triangle1);
            result.push_back(triangle2);
        }
    }
    
    return result;
}

// 绘制三角形
void Renderer::DrawTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, Shader* shader)
{
    if (!shader) return;  // 安全检查
    
    // 1.准备顶点着色器输入
    VertexShaderInput vs_in1;
    vs_in1.position = v1.pos;
    vs_in1.color = v1.color;
    vs_in1.normal = v1.normal;
    vs_in1.texcoord = v1.texcoord;
    vs_in1.modelMatrix = m_modelMatrix;
    vs_in1.viewMatrix = m_viewMatrix;
    vs_in1.projMatrix = m_projMatrix;
    
    VertexShaderInput vs_in2;
    vs_in2.position = v2.pos;
    vs_in2.color = v2.color;
    vs_in2.normal = v2.normal;
    vs_in2.texcoord = v2.texcoord;
    vs_in2.modelMatrix = m_modelMatrix;
    vs_in2.viewMatrix = m_viewMatrix;
    vs_in2.projMatrix = m_projMatrix;
    
    VertexShaderInput vs_in3;
    vs_in3.position = v3.pos;
    vs_in3.color = v3.color;
    vs_in3.normal = v3.normal;
    vs_in3.texcoord = v3.texcoord;
    vs_in3.modelMatrix = m_modelMatrix;
    vs_in3.viewMatrix = m_viewMatrix;
    vs_in3.projMatrix = m_projMatrix;
    
    // 2.执行顶点着色器
    VertexOutput vs_out1 = shader->VertexShader(vs_in1);
    VertexOutput vs_out2 = shader->VertexShader(vs_in2);
    VertexOutput vs_out3 = shader->VertexShader(vs_in3);
    
    // 3.执行背面剔除（如果启用）
    if (m_cullMode != Renderer::CullMode::CULL_NONE) {
        // 计算三角形法向量（使用叉积）
        Vector3f edge1 = Vector3f(vs_out2.worldPos - vs_out1.worldPos);
        Vector3f edge2 = Vector3f(vs_out3.worldPos - vs_out1.worldPos);
        Vector3f normal = Vector3f::cross(edge1, edge2).normalize();
        
        // 计算视线方向（从三角形中心点指向相机）
        Vector3f triangleCenter = (vs_out1.worldPos + vs_out2.worldPos + vs_out3.worldPos) / 3.0f;
        Vector3f viewDir = m_viewPosition - triangleCenter;
        
        // 判断三角形是否背向观察者（法向量与视线方向点积为负）
        float dotProduct = Vector3f::dot(normal, viewDir);
        
        // 根据绘制模式和面的朝向判断是否需要剔除
        bool isFrontFacing;
        if (m_frontFace == Renderer::FrontFace::COUNTER_CLOCKWISE) {
            // 逆时针为正面（叉积得到的法线朝外）
            isFrontFacing = (dotProduct > EPSILON);
        } else {
            // 顺时针为正面（叉积得到的法线朝内）
            isFrontFacing = (dotProduct < -EPSILON);
        }
        
        // 根据剔除模式决定是否剔除三角形
        if ((m_cullMode == Renderer::CullMode::CULL_BACK && !isFrontFacing) ||
            (m_cullMode == Renderer::CullMode::CULL_FRONT && isFrontFacing)) {
            // 需要剔除，跳过渲染
            return;
        }
    }
    
    // 4.执行近平面裁剪
    auto clippedTriangles = ClipTriangleAgainstNearPlane(vs_out1, vs_out2, vs_out3);
    
    // 如果三角形完全被裁剪掉，则跳过渲染
    if (clippedTriangles.empty()) {
        return;
    }
    
    // 对每个裁剪后的三角形进行渲染
    for (const auto& triangle : clippedTriangles) {
        // 获取裁剪后的三角形顶点
        const VertexOutput& clipVs_out1 = triangle[0];
        const VertexOutput& clipVs_out2 = triangle[1];
        const VertexOutput& clipVs_out3 = triangle[2];
        
        // 保存原始W值用于透视校正插值
        float w1 = clipVs_out1.position.w;
        float w2 = clipVs_out2.position.w;
        float w3 = clipVs_out3.position.w;
        
        // 创建副本用于透视除法和视口变换
        VertexOutput screenVs_out1 = clipVs_out1;
        VertexOutput screenVs_out2 = clipVs_out2;
        VertexOutput screenVs_out3 = clipVs_out3;
        
        // 透视除法和视口变换
        ProcessVertexOutput(screenVs_out1);
        ProcessVertexOutput(screenVs_out2);
        ProcessVertexOutput(screenVs_out3);
        
        // 计算三角形的边界盒
        int minX = (std::max)(0, (int)std::floor((std::min)({screenVs_out1.position.x, screenVs_out2.position.x, screenVs_out3.position.x})));
        int maxX = (std::min)(m_width - 1, (int)std::ceil((std::max)({screenVs_out1.position.x, screenVs_out2.position.x, screenVs_out3.position.x})));
        int minY = (std::max)(0, (int)std::floor((std::min)({screenVs_out1.position.y, screenVs_out2.position.y, screenVs_out3.position.y})));
        int maxY = (std::min)(m_height - 1, (int)std::ceil((std::max)({screenVs_out1.position.y, screenVs_out2.position.y, screenVs_out3.position.y})));
        
        // 5. 决定MipMap级别
        Vector2f v1Pos(screenVs_out1.position.x, screenVs_out1.position.y);
        Vector2f v2Pos(screenVs_out2.position.x, screenVs_out2.position.y);
        Vector2f v3Pos(screenVs_out3.position.x, screenVs_out3.position.y);
        
        // 计算纹理坐标导数(边缘)
        Vector2f edge12 = v2Pos - v1Pos;
        Vector2f edge13 = v3Pos - v1Pos;
        Vector2f texEdge12 = screenVs_out2.texcoord - screenVs_out1.texcoord;
        Vector2f texEdge13 = screenVs_out3.texcoord - screenVs_out1.texcoord;
        float det = edge12.x * edge13.y - edge12.y * edge13.x;
        float invDet = (abs(det) < EPSILON) ? 1.0f : (1.0f / det);
        
        // 计算屏幕空间对纹理坐标的导数(偏导数)
        float dudx = (edge13.y * texEdge12.x - edge12.y * texEdge13.x) * invDet;
        float dvdx = (edge13.y * texEdge12.y - edge12.y * texEdge13.y) * invDet;
        float dudy = (edge12.x * texEdge13.x - edge13.x * texEdge12.x) * invDet;
        float dvdy = (edge12.x * texEdge13.y - edge13.x * texEdge12.y) * invDet;
        
        // 计算导数的大小，这将用于选择MipMap级别
        float duvdx = std::sqrt(dudx * dudx + dvdx * dvdx);
        float duvdy = std::sqrt(dudy * dudy + dvdy * dvdy);
        
        // 6. 光栅化（使用块状遍历）
        const int BLOCK_SIZE = 8;
        for (int blockY = minY; blockY <= maxY; blockY += BLOCK_SIZE) {
            for (int blockX = minX; blockX <= maxX; blockX += BLOCK_SIZE) {
                // 计算块的边界
                int endX = (std::min)(blockX + BLOCK_SIZE, maxX + 1);
                int endY = (std::min)(blockY + BLOCK_SIZE, maxY + 1);
                
                // 遍历块内的像素
                for (int y = blockY; y < endY; y++) {
                    for (int x = blockX; x < endX; x++) {
                        float pixelX = x + 0.5f;
                        float pixelY = y + 0.5f;
                        
                        float alpha, beta, gamma;
                        if (PointInTriangle(pixelX, pixelY, v1Pos, v2Pos, v3Pos, alpha, beta, gamma)) {
                            // 恢复w值
                            screenVs_out1.position.w = w1;
                            screenVs_out2.position.w = w2;
                            screenVs_out3.position.w = w3;
                            
                            // 插值
                            VertexOutput pixelVertex = InterpolateVertex(screenVs_out1, screenVs_out2, screenVs_out3, alpha, beta, gamma);
                            
                            // 深度测试
                            if (pixelVertex.position.z <= m_currentFrameBuffer->depthBuffer.GetDepth(x, y)) {
                                // 片元着色器
                                Color pixelColor = shader->FragmentShader(pixelVertex, duvdx, duvdy);
                                
                                // 写入缓冲区
                                m_currentFrameBuffer->colorBuffer.SetPixel(x, y, pixelColor);
                                m_currentFrameBuffer->depthBuffer.SetDepth(x, y, pixelVertex.position.z);
                            }
                        }
                    }
                }
            }
        }
    }
}

// 绘制网格
void Renderer::DrawMesh(const Mesh& mesh, const Matrix& modelMatrix, Shader* shader)
{
    if (!shader) return;  // 安全检查
    
    // // 保存当前模型矩阵
    // Matrix oldModelMatrix = m_modelMatrix;
    
    // 设置新的模型矩阵
    m_modelMatrix = modelMatrix;
    
    // 遍历所有三角形
    for (size_t i = 0; i < mesh.indices.size(); i++) {
        const Vector3i& index = mesh.indices[i];
        const Vertex& v1 = mesh.vertices[index.x];
        const Vertex& v2 = mesh.vertices[index.y];
        const Vertex& v3 = mesh.vertices[index.z];
        
        // 绘制三角形
        DrawTriangle(v1, v2, v3, shader);
    }
    
    // // 恢复模型矩阵
    // m_modelMatrix = oldModelMatrix;
}

// 绘制对象
void Renderer::DrawObject(const Object& object, Shader* shader)
{
    DrawMesh(object.mesh, object.GetModelMatrix(), shader);
} 