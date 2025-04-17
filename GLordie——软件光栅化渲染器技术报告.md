# GLordie——软件光栅化渲染器

## 1. 项目概述

- 项目目标：实现一个纯软件的、不依赖Direct3D或OpenGL等第三方图形库的光栅化渲染器
- 开发环境：Windows平台，除Windows.h、WindowsX.h无其他外部库，仅使用WindowsGDI绘制像素点功能，全C++实现
- 基本功能：支持3D模型渲染、纹理映射（包括生成Mipmap三线性插值）、经验模型光照等

展示如下：

![纹理](展示结果\纹理.png)

![光照](展示结果\光照.png)

![纹理+光照](展示结果\纹理+光照.png)

<video src="展示结果/演示视频.mp4"></video>

## 2. 系统架构的核心模块

### 2.1 数学库

数学库是整个渲染器的基础设施，提供了必要的数学计算支持。主要包含向量（Vector2/3/4）、矩阵（4x4变换矩阵）和数学工具库三个部分。

主要文件：`MyMath.h`、`Vector.h`、`Vector.cpp`、`Matrix.h`、`Matrix.cpp`

#### 2.1.1 向量系统

采用模板化设计实现了二维、三维和四维向量类：

```cpp
template <typename T>
class Vector2/3/4 {
    T x, y, z, w;  // Vector3/4额外包含z和w分量
};
```

向量类支持基础的加减乘除运算，以及图形学常用的点乘、叉乘（仅Vector3）、归一化、插值等操作。在实现时考虑了性能优化，例如避免重复计算：

```cpp
float Vector3<T>::magnitude() const {
    return sqrt(magnitudeSquared());  // 利用平方和缓存
}
```

同时加入了精度控制以处理浮点数计算误差：

```cpp
#define EPSILON 0.000001f
if (std::abs(scalar) < EPSILON)
    return Vector3<T>::zero;
```

#### 2.1.2 矩阵系统

实现了4x4矩阵类用于3D变换计算，采用行主序存储：

```cpp
class Matrix {
    float m[4][4];
};
```

矩阵类支持基础运算和特殊矩阵生成（单位矩阵、平移矩阵、旋转矩阵、缩放矩阵），以及透视投影和视图变换等图形学必需的变换矩阵。针对性能瓶颈，对矩阵乘法和求逆进行了优化：

```cpp
Matrix Matrix::operator*(const Matrix& matrix) const {
    Matrix result;
    // 使用局部变量缓存，展开循环减少分支预测失败
    const float a00 = m[0][0], a01 = m[0][1];
    result.m[0][0] = a00 * b00 + a01 * b10 + a02 * b20 + a03 * b30;
    // ...
}
```

补充：之前用的高斯约旦求矩阵逆的方法没有考虑矩阵不可逆的情况，导致在特定旋转角度（如90度、270度）下会造成光的闪烁.

#### 2.1.3 数学工具库

提供了渲染器所需的各种数学工具函数，包括角度与弧度转换、数值限制、线性插值等：

```cpp
static inline float toRadians(float degrees) {
    return degrees * (PI / 180.0f);
}

inline static float clamp(float value, float min, float max) {
    return value < min ? min : (value > max ? max : value);
}
```

改进空间：可以引入SIMD指令集优化向量/矩阵运算，添加四元数支持优化旋转计算，使用查找表优化三角函数计算等。

### 2.2 渲染管线

渲染管线是软件渲染器的核心部分，实现了从3D模型到2D图像的转换过程。主要包含纹理读取创建和采样、渲染管线流程部分。

主要文件：`Texture.h`、`Texture.cpp`、`Renderer.h`、`Renderer.cpp`

#### 2.2.1 纹理采样

实现了完整的纹理采样系统，支持包含三线性过滤的多种过滤模式和环绕模式：

```cpp
class Texture {
    // 支持三种过滤模式
    enum class TextureFilterMode {
        NEAREST,    // 最近邻过滤
        BILINEAR,   // 双线性过滤
        TRILINEAR   // 三线性过滤（配合Mipmap）
    };
    
    // 支持三种环绕模式
    enum class TextureWrapMode {
        REPEAT,     // 重复
        CLAMP,      // 钳制
        MIRROR      // 镜像
    };
};
```

为了提高渲染质量，实现了Mipmap相关技术，包括生成Mipmap，根据导数分级采样Mipmap并进行三线性插值：

```cpp
Color Texture::Sample(float u, float v, float dudx, float dvdy) const {
    if (hasMipmaps && filterMode == TextureFilterMode::TRILINEAR) {
        float level = CalculateMipmapLevel(dudx, dvdy);
        return TrilinearSample(u, v, level);
    }
    return Sample(u, v);
}
```

补充：GDI+使用BGRA格式，读取.jpg图片后，需要交换红蓝通道（好坑啊）。

还实现了F键在三种过滤模式之间切换，T键在三种环绕模式切换功能。

#### 2.2.2 裁剪和背面剔除

实现了近平面裁剪以避免渲染近平面后的物体，当三角形面与近平面相交时，根据不同情况生成新的小三角形：

```cpp
std::vector<std::array<VertexOutput, 3>> Renderer::ClipTriangleAgainstNearPlane(/*...*/) {
    // 计算每个顶点是否在近平面后面
    bool v1Inside = v1.position.w >= NEAR_PLANE;
    bool v2Inside = v2.position.w >= NEAR_PLANE;
    bool v3Inside = v3.position.w >= NEAR_PLANE;
    
    // 根据顶点位置生成新的三角形
    if (insideCount == 1) {
        // 生成一个新三角形
    } else if (insideCount == 2) {
        // 生成两个新三角形
    }
}
```

有三种剔除模式：

```cpp
// 剔除模式
enum class CullMode {
    CULL_BACK,      // 剔除背面（默认）
    CULL_FRONT,     // 剔除正面 
    CULL_NONE       // 双面绘制
};
```

剔除通过判断三角形法线和视线方向的夹角实现：

```cpp
if (m_cullMode != CullMode::CULL_NONE) {
    Vector3f normal = Vector3f::cross(edge1, edge2).normalize();
    Vector3f viewDir = m_viewPosition - triangleCenter;
    float dotProduct = Vector3f::dot(normal, viewDir);
    
    bool isFrontFacing = (m_frontFace == FrontFace::COUNTER_CLOCKWISE) 
        ? (dotProduct > EPSILON) 
        : (dotProduct < -EPSILON);
        
    if ((m_cullMode == CullMode::CULL_BACK && !isFrontFacing) ||
        (m_cullMode == CullMode::CULL_FRONT && isFrontFacing)) {
        return; // 剔除该三角形
    }
}
```

补充：可通过B键切换三种剔除模式，还实现了N键切换法线反转、V键切换面片翻转等额外功能。


#### 2.2.3 光栅化

采用块状扫描的方式进行三角形光栅化，提高缓存命中率：

```cpp
const int BLOCK_SIZE = 8;
for (int blockY = minY; blockY <= maxY; blockY += BLOCK_SIZE) {
    for (int blockX = minX; blockX <= maxX; blockX += BLOCK_SIZE) {
        // 遍历块内像素
        for (int y = blockY; y < endY; y++) {
            for (int x = blockX; x < endX; x++) {
                if (PointInTriangle(x + 0.5f, y + 0.5f, /*...*/)) {
                    // 处理像素
                }
            }
        }
    }
}
```

使用重心坐标进行顶点属性插值：

```cpp
VertexOutput InterpolateVertex(/*...*/) {
    // 透视校正插值
    float correctionW1 = w1 / v1.position.w;
    float correctionW2 = w2 / v2.position.w;
    float correctionW3 = w3 / v3.position.w;
    float normalizer = 1.0f / (correctionW1 + correctionW2 + correctionW3);
    
    // 插值各种顶点属性（位置、颜色、法线、纹理坐标等）
    result.color = (/*...*/) * normalizer;
}
```

#### 2.2.4 深度测试

实现了深度缓冲以解决可见性问题：

```cpp
if (pixelVertex.position.z <= m_currentFrameBuffer->depthBuffer.GetDepth(x, y)) {
    // 深度测试通过，更新颜色和深度缓冲
    Color pixelColor = shader->FragmentShader(pixelVertex, duvdx, duvdy);
    m_currentFrameBuffer->colorBuffer.SetPixel(x, y, pixelColor);
    m_currentFrameBuffer->depthBuffer.SetDepth(x, y, pixelVertex.position.z);
}
```

优化空间：可以引入多线程处理、早期深度测试等技术进一步提升性能。

### 2.3 着色系统

着色系统采用了灵活的基类设计模式，通过虚函数实现了可扩展的着色器框架，支持多种着色模型，目前包括：普通颜色着色器ColorShader、Phong经验模型光照着色器PhongShader、BlinnPhong经验模型光照着色器BlinnPhongShader、简单纹理着色器TextureShader、纹理-光照着色器TextureBlinnPhongShader。

主要文件：`Shader.h`、`Shader.cpp`

#### 2.3.1 着色器框架设计

基类设计采用了顶点着色器和片元着色器分离的结构：

```cpp
class Shader {
public:
    // 顶点着色器接口
    virtual VertexOutput VertexShader(const VertexShaderInput& input) = 0;
    // 片元着色器接口
    virtual Color FragmentShader(const VertexOutput& input, float dudx, float dvdy) = 0;
    
    // 光照参数配置
    void SetLight(const LightParams& light) { m_light = light; }

protected:
    LightParams m_light;  // 光照参数
};
```

着色器输入输出结构设计：

```cpp
struct VertexShaderInput {
    Vector4f position;    // 顶点位置
    Vector4f color;       // 顶点颜色
    Vector3f normal;      // 顶点法线
    Vector2f texcoord;    // 纹理坐标
    Matrix modelMatrix;   // 模型矩阵
    Matrix viewMatrix;    // 视图矩阵
    Matrix projMatrix;    // 投影矩阵
};

struct VertexOutput {
    Vector4f position;    // 裁剪空间位置
    Vector4f color;       // 插值颜色
    Vector3f normal;      // 世界空间法线
    Vector2f texcoord;    // 纹理坐标
    Vector3f worldPos;    // 世界空间位置
};
```

#### 2.3.2 着色器实现

##### 2.3.2.1 颜色着色器

最基础的着色器，直接使用顶点颜色：

```cpp
class ColorShader : public Shader {
    virtual Color FragmentShader(const VertexOutput& input, float dudx, float dvdy) override {
        return Color(input.color.x, input.color.y, input.color.z, input.color.w);
    }
};
```

##### 2.3.2.2 Blinn-Phong着色器

实现了完整的Blinn-Phong光照模型：

```cpp
class BlinnPhongShader : public Shader {
    virtual Color FragmentShader(const VertexOutput& input, float dudx, float dvdy) override
    {
        // 基础颜色
        Color baseColor(input.color.x, input.color.y, input.color.z, input.color.w);
        
        // 环境光
        Color ambient = m_light.ambient * baseColor;
        
        // 漫反射
        Vector3f lightDir = (m_light.position - input.worldPos).normalize();
        float diff = (std::max)(0.0f, Vector3f::dot(input.normal, lightDir));
        Color diffuse = m_light.diffuse * baseColor * diff;
        
        // Blinn-Phong高光计算 (不同于Phong的反射计算)
        Vector3f viewDir = (m_viewPosition - input.worldPos).normalize();
        Vector3f halfDir = (lightDir + viewDir).normalize(); // 半程向量
        float spec = (std::pow)((std::max)(0.0f, Vector3f::dot(input.normal, halfDir)), m_shininess);
        Color specular = m_light.specular * spec;
        
        // 最终颜色
        Color finalColor = ambient + diffuse + specular;
        finalColor.a = baseColor.a; // 保持原透明度
        
        // 确保颜色分量在[0,1]范围内
        finalColor.r = clamp01(finalColor.r);
        finalColor.g = clamp01(finalColor.g);
        finalColor.b = clamp01(finalColor.b);
        
        return finalColor;
    }
};
```

##### 2.3.2.3 纹理-光照着色器

支持纹理采样和Mipmap：

```cpp
class TexturedBlinnPhongShader : public Shader {
    virtual Color FragmentShader(const VertexOutput& input, float dudx, float dvdy) override {
        // 纹理采样
        Color texColor = m_texture->Sample(input.texcoord.x, input.texcoord.y, dudx, dvdy);
        
        // 光照计算（与Blinn-Phong相同）
        Color ambient = m_light.ambient * texColor;
        // ... 漫反射和高光计算 ...
        
        // 最终颜色
        Color finalColor = ambient + (diffuse + specular) * m_light.intensity;

        finalColor.a = baseColor.a; // 保持原透明度
        
        // 确保颜色分量在[0,1]范围内
        finalColor.r = clamp01(finalColor.r);
        finalColor.g = clamp01(finalColor.g);
        finalColor.b = clamp01(finalColor.b);
        
        return finalColor;
    }
};
```

优化空间：添加更多特效着色器（如卡通渲染、环境光遮蔽等），使用SIMD指令集优化光照计算提升性能。

### 2.4 缓冲区管理

缓冲区管理采用单例模式设计，实现了双缓冲机制和深度测试功能，是渲染器实现稳定输出和正确三维效果的关键部分。

#### 2.4.1 缓冲区基础架构

```cpp
// 基础缓冲区类
class Buffer {
protected:
    const DWORD BUFFER_SIZE;
    unsigned char* buffer;   // 缓冲区数据
    unsigned int width;      // 宽度
    unsigned int height;     // 高度
};

// 帧缓冲区组合
class FrameBuffer {
public:
    ColorBuffer colorBuffer;    // 颜色缓冲
    DepthBuffer depthBuffer;   // 深度缓冲
};
```

#### 2.4.2 颜色缓冲实现

颜色缓冲支持RGBA四通道色彩，实现了颜色混合：

```cpp
class ColorBuffer : public Buffer {
public:
    void SetPixel(unsigned int x, unsigned int y, const Color& color) {
        if (x >= width || y >= height)
            return;

        unsigned int index = (y * width + x) * 4;
        buffer[index]     = static_cast<BYTE>(color.r * 255.0f);     // R
        buffer[index + 1] = static_cast<BYTE>(color.g * 255.0f);     // G
        buffer[index + 2] = static_cast<BYTE>(color.b * 255.0f);     // B
        buffer[index + 3] = static_cast<BYTE>(color.a * 255.0f);     // A
    }
};
```

#### 2.4.3 深度缓冲实现

深度缓冲使用浮点数存储深度值，用于可见性判断：

```cpp
class DepthBuffer {
public:
    void SetDepth(unsigned int x, unsigned int y, float depth) {
        depth = clamp01(depth);  // 将深度值限制在[0,1]范围内
        if (x >= width || y >= height)
            return;
        buffer[y * width + x] = depth;
    }

    float GetDepth(unsigned int x, unsigned int y) const {
        if (x >= width || y >= height)
            return 1.0f;
        return buffer[y * width + x];
    }
};
```

#### 2.4.4 双缓冲管理

使用单例模式实现双缓冲管理器，避免画面撕裂：

```cpp
class BufferManager {
private:
    static BufferManager* s_instance;
    FrameBuffer m_frontBuffer;         // 前置缓冲区
    FrameBuffer m_backBuffer;          // 后置缓冲区
    Color m_backgroundColorObj;        // 背景颜色

public:
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
};
```

以上三个函数暴露给Renderer调用，如下：

```cpp
void Renderer::SwapBuffers(HDC hdc)
{
    // 交换缓冲区
    m_bufferManager->SwapBuffers();
    
    // 将前缓冲区呈现到目标设备上下文
    m_bufferManager->PresentToHDC(hdc);
    
    // 获取新的后缓冲区用于下一帧绘制
    m_currentFrameBuffer = m_bufferManager->GetBackBuffer();
}
```

#### 2.4.5 性能优化特点

1. 内存管理优化：
   - 预分配固定大小缓冲区
   - 使用连续内存布局
   - 避免频繁的内存分配/释放

2. 访问优化：
   - 内联简单操作
   - 边界检查避免越界访问
   - 使用指针交换而非内存拷贝

3. 渲染优化：
   - 双缓冲消除画面撕裂
   - 深度测试避免过度绘制
   - 块状访问提高缓存命中率

缓冲区管理系统为渲染器提供了稳定的图像输出基础，通过双缓冲机制避免了画面撕裂，深度测试确保了正确的三维效果显示。

### 2.5 资源管理系统

资源管理系统负责处理外部资源的加载和处理，包括纹理、3D模型和材质系统，为渲染器提供必要的资源支持。

主要文件：`Texture.h`、`Texture.cpp`、`ObjFileReader.h`、`ObjFileReader.cpp`、`Object.h`、`Object.cpp`

#### 2.5.1 纹理系统

支持多种格式纹理加载和处理，实现了完整的纹理过滤和Mipmap生成：

```cpp
class Texture {
public:
    // 支持多种格式加载
    bool LoadFromFile(const std::string& path);
    bool LoadFromBMP(const char* path);
    bool LoadFromJPG(const char* path);

    // 纹理过滤和环绕模式
    TextureFilterMode filterMode;  // NEAREST/BILINEAR/TRILINEAR
    TextureWrapMode wrapMode;      // REPEAT/CLAMP/MIRROR

    // 采样纹理
    Color Sample(float u, float v) const;
    Color Sample(float u, float v, float dudx, float dvdy) const; // 带有导数的采样（用于Mipmap）
    
    // Mipmap相关函数
    void GenerateMipmaps();
    void ClearMipmaps();
    bool SaveMipmapsToJPG(const std::string& basePath, int quality = 90) const;
    
    // 生成测试纹理（如棋盘格纹理、渐变纹理等）
    static Texture CreateCheckerboard(int width, int height, int checkSize, const Color& color1, const Color& color2);
    static Texture CreateGradient(int width, int height, const Color& startColor, const Color& endColor, bool horizontal = true);
    static Texture CreateCircle(int size, const Color& circleColor, const Color& backgroundColor);
};
```

#### 2.5.2 OBJ模型加载

实现了OBJ格式3D模型的加载和处理：

```cpp
class ObjFileReader {
public:
     // 从文件加载OBJ模型，返回构建好的Object对象
    static Object LoadFromFile(const std::string& filePath);
    
    // 从文件加载OBJ模型，可以指定是否翻转法线和面片环绕顺序
    static Object LoadFromFileWithOptions(const std::string& filePath, 
                                         bool flipNormals = false, 
                                         bool flipFaces = false);
    
    // 从文件加载OBJ模型，并构建Mesh
    static Mesh LoadMeshFromFile(const std::string& filePath);
    
    // 从文件加载OBJ模型，可以指定是否翻转法线和面片环绕顺序
    static Mesh LoadMeshFromFileWithOptions(const std::string& filePath, 
                                          bool flipNormals = false, 
                                          bool flipFaces = false);
    
private:
    // 解析OBJ文件中的一行数据
    static void ParseLine(const std::string& line, 
                   std::vector<Vector3f>& positions,
                   std::vector<Vector2f>& texcoords,
                   std::vector<Vector3f>& normals,
                   std::vector<int>& positionIndices,
                   std::vector<int>& texcoordIndices,
                   std::vector<int>& normalIndices);
    
    // 从已解析的数据构建Mesh
    static Mesh BuildMesh(const std::vector<Vector3f>& positions,
                   const std::vector<Vector2f>& texcoords,
                   const std::vector<Vector3f>& normals,
                   const std::vector<int>& positionIndices,
                   const std::vector<int>& texcoordIndices,
                   const std::vector<int>& normalIndices,
                   bool flipNormals = false,
                   bool flipFaces = false);

    void ObjFileReader::ParseLine(const std::string& line, 
                            std::vector<Vector3f>& positions,
                            std::vector<Vector2f>& texcoords,
                            std::vector<Vector3f>& normals,
                            std::vector<int>& positionIndices,
                            std::vector<int>& texcoordIndices,
                            std::vector<int>& normalIndices) {
    // 跳过空行和注释行
    if (line.empty() || line[0] == '#')
        return;
    
    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix;
    
    if (prefix == "g" || prefix == "usemtl" || prefix == "mtllib" || prefix == "s") {
        // 忽略这些命令
        return;
    }
    else if (prefix == "v") {
        // 顶点位置 (v x y z)
        float x, y, z;
        iss >> x >> y >> z;
        positions.push_back(Vector3f(x, y, z));
    }
    else if (prefix == "vt") {
        // 纹理坐标 (vt u v)
        float u, v;
        iss >> u >> v;
        // 翻转V坐标以匹配DirectX/OpenGL纹理坐标系统
        texcoords.push_back(Vector2f(u, 1.0f - v));
    }
    else if (prefix == "vn") {
        // 法线 (vn x y z)
        float x, y, z;
        iss >> x >> y >> z;
        normals.push_back(Vector3f(x, y, z));
    }
    else if (prefix == "f") {
        // 面 (f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3)
        std::string vertexData;
        std::vector<int> facePositionIndices;
        std::vector<int> faceTexcoordIndices;
        std::vector<int> faceNormalIndices;
        // 省略......
    }
};
```

补充：很多.obj模型的顶点并不是都放在一起的，而是分段放在文件中，如果只能处理如犹他茶壶模型这种顶点都放在一起的.obj模型，处理别的大部分模型会导致很多面片的缺失。不仅如此，f后面跟的格式不一定是完整的(f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3)，可能会有省略，具体可以用文本编辑器看.obj模型内容。还有一点最重要的是，需要翻转纹理V坐标，不然纹理没法正确贴在物体上（好坑啊）。

#### 2.5.3 物体系统

实现了基础的物体系统，从顶点到网格到物体逐级封装：

```cpp
// 顶点类
class Vertex
{
public:
    Vector4f pos; // 位置
    Vector4f color; // 颜色
    Vector3f normal; // 法线
    Vector2f texcoord; // 纹理坐标
}

// 网格类
class Mesh
{
public:
    std::vector<Vertex> vertices; // 顶点数组
    std::vector<Vector3i> indices; // 顶点索引数组
}

// 材质类
class Material {
public:
    Color ambient;      // 环境光颜色
    Color diffuse;      // 漫反射颜色
    Color specular;     // 镜面反射颜色
    float shininess;    // 高光指数
    float opacity;      // 不透明度

    Material() 
        : ambient(Color::white)
        , diffuse(Color::white)
        , specular(Color::white)
        , shininess(32.0f)
        , opacity(1.0f) 
    {}
};

// 对象类整合了网格、材质和变换
class Object {
public:
    Mesh mesh;              // 网格数据
    Material material;      // 材质
    Transformer transform;  // 变换信息

    // 包围体计算
    void CalculateBoundingSphere(Vector3f& center, float& radius) const {
        mesh.CalculateBoundingSphere(center, radius);
        // 考虑变换的影响
        Matrix modelMatrix = GetModelMatrix();
        Vector4f centerVec(center.x, center.y, center.z, 1.0f);
        centerVec = modelMatrix * centerVec;
        center = Vector3f(centerVec.x, centerVec.y, centerVec.z);
        // 考虑缩放对半径的影响
        float maxScale = std::max({transform.scale.x, 
                                 transform.scale.y, 
                                 transform.scale.z});
        radius *= maxScale;
    }
};
```

补充：实际上Material材质类没有使用，每次创建物体都是构造一个空的材质类，但是材质类是PBR光照实现的基础，之后要实现PBR光照必须完善。（应该是一个材质对应一个Shader，物体上可以有很多个材质，.mtl文件是必须的，也必须解析.obj文件中的usemtl指令）

#### 2.5.4 资源管理特点

1. 功能完整性：
   - 支持主流图片格式（BMP、JPG）
   - 支持标准OBJ模型格式

2. 优化措施：
   - 纹理Mipmap生成
   - 模型数据预处理
   - 包围体计算加速

3. 扩展性：
   - 易于添加新的资源格式支持
   - 可扩展的材质系统
   - 灵活的对象管理

优化空间：添加资源缓存系统、支持更多文件格式、实现材质编辑器等。


## 3. 已知缺陷

尽管项目成功实现了核心的软件光栅化渲染功能，但仍存在一些已知缺陷和待改进之处：

### 3.1 渲染功能限制

- 渲染管线相对基础，缺少现代图形API中的几何着色器（Geometry Shader）和曲面细分（Tessellation）阶段，限制了可实现的几何效果复杂度。
- 不支持实例化渲染（Instancing），绘制大量相同物体效率较低。
- 裁剪仅实现了近平面裁剪，未实现完整的视锥体裁剪（左右上下远平面），可能导致部分性能浪费和潜在的渲染错误。
- 缺乏抗锯齿（Anti-aliasing）处理，渲染结果边缘可能存在锯齿。
- 透明度处理不完善，虽然支持Alpha通道，但未实现正确的混合（Blending）和排序，渲染半透明物体效果不正确，还可考虑实现OIT技术([LearnOpenGL - Introduction](https://learnopengl.com/Guest-Articles/2020/OIT/Introduction))优化半透明物体渲染。
- 未实现任何阴影（Shadows）渲染技术。
- 缺少后处理（Post-processing）框架，难以实现诸如泛光（Bloom）、景深（Depth of Field）等屏幕空间效果。

### 3.2 **性能瓶颈**

- 整个渲染过程是单线程执行的，无法利用现代多核CPU的优势，是主要的性能瓶颈。
- 数学库（向量、矩阵运算）未使用SIMD指令集进行优化，计算密集型操作效率有提升空间。
- 光栅化和像素处理阶段存在优化空间，例如可以探索更高级的光栅化算法或进一步优化内存访问模式。
- 频繁的函数调用和数据拷贝也可能带来一定的性能开销。

### 3.3 **材质与光照**

- 光照模型比较基础，仅支持简单的点光源和Blinn-Phong模型，缺乏对更复杂光照现象（如全局光照、基于物理的渲染 PBR）的支持。
- 材质系统较为简单，仅支持颜色和基础光照参数，缺少对法线贴图、高光贴图等现代材质属性的支持。
- 实际上Material材质类没有使用，每次创建物体都是构造一个空的材质类，但是材质类是PBR光照实现的基础，之后要实现PBR光照必须完善。（应该是一个材质对应一个Shader，物体上可以有很多个材质，.mtl文件是必须的，也必须解析.obj文件中的usemtl指令）

### 3.4 **其他**

- 资源加载和错误处理机制相对简单，健壮性有待提高。
- 物体的缩放、旋转等操作均依赖帧数，帧数越高，物体旋转地越快，应该实现类似fixedUpdate方法处理固定帧率。
- 场景编辑器这块实现非常简陋，一切物体、材质、Shader都以全局变量的方式放在`main.cpp`中，之后需要封装。
- 直接使用GDI进行文本渲染和窗口呈现，限制了跨平台能力和部分性能。

## 4. 性能分析

- CPU：Intel Core i7-13790F (13th Gen)

- 内存：64GB

- 显卡：NVIDIA GeForce RTX 4070

但是，本项目根本没有调用GPU，且占内存微乎其微（70MB），因此内存和显卡性能与本项目无关。

使用猫猫模型总共运行1分40秒，中间每隔20秒切换不同Shader、纹理采样方式、剔除模式，最终得到的性能分析报告如下：

![性能分析](展示结果\性能分析.png)

![函数调用树](展示结果\函数调用树.png)

### 4.1 主要瓶颈

#### 4.1.1 光栅化阶段的三角形绘制、像素处理

#### 4.1.2 纹理采样的双线性插值、三线性插值

#### 4.1.3 向量归一化操作、矩阵乘法、矩阵求逆等矩阵运算

实际上，在单线程情况下，已经将矩阵乘法、矩阵求逆等运算使用循环展开等技术尽力优化了，再优化就必须上多线程技术。

### 4.2 测试数据

#### 4.2.1 模型复杂度、着色器、剔除模式、纹理过滤方式对性能的影响

在物体所占屏幕比例差不多的情况下：

<img src="展示结果\屏幕比例示意1.png" alt="屏幕比例示意1" style="zoom:60%;" /><img src="展示结果\屏幕比例示意2.png" alt="屏幕比例示意2" style="zoom:60%;" />

不同模型复杂度、不同着色器、不同剔除模式、不同纹理过滤方式等情况下的渲染帧率如下：

| 模型     | 顶点数 | 三角形数 | 材质大小  | 着色器类型 | 剔除模式 | 过滤方式   | 平均帧率 (帧/秒) |
| -------- | ------ | -------- | --------- | ---------- | -------- | ---------- | ---------------- |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 纹理-光照  | 背面剔除 | 三线性插值 | 42               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 纹理-光照  | 不剔除   | 三线性插值 | 33               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 纹理-光照  | 背面剔除 | 双线性插值 | 43               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 纹理-光照  | 不剔除   | 双线性插值 | 34               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 纹理-光照  | 背面剔除 | 最近邻     | 44               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 纹理-光照  | 不剔除   | 最近邻     | 35               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 纹理       | 背面剔除 | 三线性插值 | 57               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 纹理       | 不剔除   | 三线性插值 | 43               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 纹理       | 背面剔除 | 双线性插值 | 59               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 纹理       | 不剔除   | 双线性插值 | 45               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 纹理       | 背面剔除 | 最近邻     | 61               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 纹理       | 不剔除   | 最近邻     | 47               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 光照       | 背面剔除 | 三线性插值 | 46               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 光照       | 不剔除   | 三线性插值 | 36               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 光照       | 背面剔除 | 双线性插值 | 46               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 光照       | 不剔除   | 双线性插值 | 36               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 光照       | 背面剔除 | 最近邻     | 46               |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 光照       | 不剔除   | 最近邻     | 36               |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 纹理-光照  | 背面剔除 | 三线性插值 | 132              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 纹理-光照  | 不剔除   | 三线性插值 | 108              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 纹理-光照  | 背面剔除 | 双线性插值 | 165              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 纹理-光照  | 不剔除   | 双线性插值 | 132              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 纹理-光照  | 背面剔除 | 最近邻     | 175              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 纹理-光照  | 不剔除   | 最近邻     | 136              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 纹理       | 背面剔除 | 三线性插值 | 177              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 纹理       | 不剔除   | 三线性插值 | 141              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 纹理       | 背面剔除 | 双线性插值 | 249              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 纹理       | 不剔除   | 双线性插值 | 192              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 纹理       | 背面剔除 | 最近邻     | 283              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 纹理       | 不剔除   | 最近邻     | 214              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 光照       | 背面剔除 | 三线性插值 | 194              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 光照       | 不剔除   | 三线性插值 | 158              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 光照       | 背面剔除 | 双线性插值 | 196              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 光照       | 不剔除   | 双线性插值 | 159              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 光照       | 背面剔除 | 最近邻     | 194              |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 光照       | 不剔除   | 最近邻     | 158              |

可以得出以下几个结论：

1. **模型复杂度影响性能**:

   **模型复杂度**越高，性能越低。

   猫猫模型的顶点数和三角形数相对较多（21,728个顶点和70,576个三角形），在各种情况下帧率普遍较低，汽车模型有较小的顶点和三角形数（3,534个顶点和1,178个三角形），在各种情况下帧率普遍较高。

2. **着色器类型的影响**:

   **着色器**越复杂，性能越低。复杂程度：纹理-光照着色器 >  纹理着色器 > 光照着色器

3. **剔除方式的影响**:

   **背面剔除** 选项普遍提供了较高帧率，尤其是在使用双线性和三线性插值时，显示出剔除不必要的面片对性能的积极影响。

4. **过滤方式的影响**:

   **过滤方式** 的复杂程度也显著影响了性能，过滤方式越复杂，性能越低。复杂程度：三线性插值 > 双线性 > 最近邻。

#### 4.2.2 渲染物体所占屏幕比例对性能的影响

这里仅做简单测试，在渲染物体占屏幕比例较大时，帧率会显著下降。

例如，在以下情况下增大猫猫模型，并调整相机高度：

| 模型     | 顶点数 | 三角形数 | 材质大小  | 着色器类型 | 剔除模式 | 过滤方式   | 平均帧率 (帧/秒) |
| -------- | ------ | -------- | --------- | ---------- | -------- | ---------- | ---------------- |
| 猫猫模型 | 21,728 | 70,576   | 1024x1024 | 纹理-光照  | 背面剔除 | 三线性插值 | 42               |

![较大屏幕比例示意1](展示结果\较大屏幕比例示意1.png)

平均帧率下降到22帧/秒。

在以下情况下增大汽车模型，并调整相机高度：

| 模型     | 顶点数 | 三角形数 | 材质大小  | 着色器类型 | 剔除模式 | 过滤方式   | 平均帧率 (帧/秒) |
| -------- | ------ | -------- | --------- | ---------- | -------- | ---------- | ---------------- |
| 汽车模型 | 3,534  | 1,178    | 1080x1080 | 纹理-光照  | 背面剔除 | 三线性插值 | 132              |

![较大屏幕比例示意2](展示结果\较大屏幕比例示意2.png)

平均帧率下降到29帧/秒。

## 5. 未来展望

### 5.1 功能扩展

- 支持阴影渲染
- 抗锯齿方法(MSAA、SSAA等)
- Early-Z
- 半透明渲染(AlphaTest、AlphaBlend)
- 法线贴图、高光贴图
- 多个纹理映射到一个物体
- 材质(Material)系统
- 实现后处理效果(黑白滤镜、边缘检测、泛光Bloom等)
- 描边效果
- 天空盒
- PBR光照
- 实现几何着色器
- 添加粒子系统

### 5.2 性能优化

- 多线程渲染支持
- SIMD指令集优化
- 层次包围盒加速(BVH、AABB等)
- 渲染命令排序

### 5.3 工具链完善

- 场景编辑器（这块实现非常简陋，一切物体、材质、Shader都以全局变量的方式放在`main.cpp`中，之后需要封装）
- 材质编辑器
- 性能分析工具

## 6. 项目总结

​	行文至此，这个软件光栅化渲染器项目终于要暂时告一段落了。我为项目起名GLordie，读音歌楼地，实际上就是取GL or die的合并，以展示项目不使用第三方图形库的特点。遥想上周一刚入职的时候，我对这个项目还是感觉十分困难，怀疑自己能不能在两周之内完成等想法。尽管之前已经完成过games101课程作业里的类似工程，但也只是补充其中关键函数，谈何从零开始完成整个项目，但是，在各个知乎好帖以及ai编程工具以及我当年还没遗忘的games101经验来看，我还是成功地完成了整个项目，并尽量把代码写的没那么耦合，事实上，在第四天的时候，这个项目就已经基本完成，第五天主要是修复各种恶性bug，第六天测试性能完成文档，项目达到目前的状态，已经没有什么corner case是需要藏着掖着的了，（第五天的时候还有模型旋转设置到特殊角度90or270度会出现光线闪烁、模型贴图颜色跟实际贴图不一样等等corner case）基本上都是按照预想运行。总而言之，项目超额完成了我的预期，之后再完善项目只需要再增加各种功能就行了，底层的代码和框架基本都定死，不会有什么问题了。

​	感谢公司提供的办公条件和资源，以及这个在上周的我看来几乎不可能完成的要求。

​	短短六天，收获颇丰。

​																															——XYH

​																									写于xx娱乐，2025年x月xx日17：37分
