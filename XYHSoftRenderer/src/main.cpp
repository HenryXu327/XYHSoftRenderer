#include <Windows.h>
#include <WindowsX.h>  // 添加此头文件以使用GET_X_LPARAM和GET_Y_LPARAM
#include <cmath>
#include <memory>
#include <string>
#include "../include/Window.h"
#include "../include/Color.h"
#include "../include/Renderer.h"
#include "../include/Shader.h"
#include "../include/Object.h"
#include "../include/MyMath.h"
#include "../include/Camera.h"
#include "../include/Texture.h"
#include "../include/ObjFileReader.h" // 添加ObjFileReader头文件

// 当前绘制模式
enum class DrawMode {
    Test2D,            // 2D测试绘制
    Shader3D,          // 3D着色器绘制
    TextureShader3D,    // 3D纹理着色器绘制
    TextureBlinnPhong3D, // 3D纹理Blinn-Phong着色器绘制
    ObjModel3D          // 3D OBJ模型显示
};
// ================================================
// 全局变量
// ================================================

// 当前绘制模式
DrawMode g_currentMode = DrawMode::ObjModel3D;

// 立方体网格
Mesh g_cubeMesh; // 添加立方体网格
Object g_cube; // 添加立方体对象

// OBJ模型
Object g_objModel; // 添加OBJ模型对象
bool g_objModelLoaded = false; // OBJ模型是否已加载

// 着色器
ColorShader g_colorShader;  // 添加ColorShader实例
PhongShader g_phongShader;  // 添加PhongShader实例
BlinnPhongShader g_blinnPhongShader; // 添加BlinnPhongShader实例
TextureShader g_textureShader; // 添加TextureShader实例
TexturedBlinnPhongShader g_texturedBlinnPhongShader; // 添加TexturedBlinnPhongShader实例

// 纹理
Texture g_texture;          // 添加纹理实例

// 相机
Camera g_camera;            // 添加相机实例

// 角度
float g_angle = 0.0f;

// 光源位置
Vector3f g_lightPosition(7.0f, 7.0f, 8.0f);
float g_lightIntensity = 1.0f;

// 鼠标控制相关变量
bool g_firstMouse = true;
float g_lastX = 0.0f;
float g_lastY = 0.0f;
float g_yaw = 0.0f;
float g_pitch = 0.0f;
float g_mouseSensitivity = 0.05f;
bool g_mouseCaptured = false;  // 是否已捕获鼠标

// 全局变量用于记录是否使用带纹理Obj着色器
bool g_useTextureShaderForObj = false;
bool g_flipNormals = false; // 是否翻转法线
bool g_flipFaces = false;   // 是否翻转面片环绕顺序
Renderer::CullMode g_cullMode = Renderer::CullMode::CULL_BACK; // 默认背面剔除

// 函数前向声明
void DrawTestPattern(Window& window);
void DrawShaderDemo(Window& window);
void DrawTextureShaderDemo(Window& window);
void DrawTextureBlinnPhongShaderDemo(Window& window);
void DrawObjModel(Window& window);
void RenderCurrentScene(Window& window);
bool InitShaderDemo(Window& window);
void UpdateCameraDirectionWithMouse(float xpos, float ypos);
void HandleKeyDown(HWND hwnd, WPARAM wParam);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//=================================
// 辅助函数
//=================================

// 创建统一的光照参数
LightParams CreateLightParams(float ambientIntensity = 0.1f, float diffuseIntensity = 0.7f) {
    LightParams light;
    light.position = g_lightPosition;
    light.ambient = Color(ambientIntensity, ambientIntensity, ambientIntensity, 1.0f);
    light.diffuse = Color(diffuseIntensity, diffuseIntensity, diffuseIntensity, 1.0f);
    light.specular = Color(1.0f, 1.0f, 1.0f, 1.0f);
    light.intensity = g_lightIntensity;
    return light;
}

// 更新所有光照着色器的参数
void UpdateLightParams(float ambientIntensity = 0.1f, float diffuseIntensity = 0.7f) {
    LightParams light = CreateLightParams(ambientIntensity, diffuseIntensity);
    g_phongShader.SetLight(light);
    g_blinnPhongShader.SetLight(light);
    g_texturedBlinnPhongShader.SetLight(light);
}

// 更新所有着色器的相机位置
void UpdateShadersViewPosition() {
    g_phongShader.SetViewPosition(g_camera.position);
    g_blinnPhongShader.SetViewPosition(g_camera.position);
    g_texturedBlinnPhongShader.SetViewPosition(g_camera.position);
}

// 更新渲染器的相机参数
void UpdateRenderer(Renderer* renderer) {
    renderer->SetViewMatrix(g_camera.GetViewMatrix());
    renderer->SetViewPosition(g_camera.position);
}

// 清空渲染缓冲区
void ClearRenderBuffers(Renderer* renderer, const Color& backgroundColor) {
    renderer->ClearBackBuffer(backgroundColor);
    renderer->ClearDepthBuffer(1.0f);
}

// 创建向量坐标字符串
std::wstring CreateVectorString(const Vector3f& vector, const std::wstring& name) {
    return name + L": (" + 
        std::to_wstring(vector.x) + L", " +
        std::to_wstring(vector.y) + L", " +
        std::to_wstring(vector.z) + L")";
}

// 绘制物体信息
void DrawObjectInfo(Renderer* renderer, const Object& object, int startY = 70) {
    std::wstring posInfo = CreateVectorString(object.transform.position, L"Position");
    std::wstring rotInfo = CreateVectorString(object.transform.rotation, L"Rotation");
    std::wstring scaleInfo = CreateVectorString(object.transform.scale, L"Scale");
    
    renderer->DrawText(10, startY, posInfo, Color::white);
    renderer->DrawText(10, startY + 20, rotInfo, Color::white);
    renderer->DrawText(10, startY + 40, scaleInfo, Color::white);
}

// 绘制相机信息
void DrawCameraInfo(Renderer* renderer, int y = 150) {
    std::wstring camPosInfo = CreateVectorString(g_camera.position, L"Camera");
    renderer->DrawText(10, y, camPosInfo, Color::white);
}

// 绘制纹理信息
void DrawTextureInfo(Renderer* renderer, const Texture& texture, int y = 170, const std::wstring& textureName = L"") {
    std::wstring textureInfo = L"Texture: " + std::to_wstring(texture.width) + L"x" + 
        std::to_wstring(texture.height) + L" pixels";
    
    if (!textureName.empty()) {
        textureInfo += L" (" + textureName + L")";
    }
    
    renderer->DrawText(10, y, textureInfo, Color::white);
}

// 绘制标准操作提示
void DrawStandardControls(Renderer* renderer, const std::wstring& currentMode, int y = 10) {
    renderer->DrawText(10, y, L"Space: Switch mode, WASD: Move camera", Color::white);
    renderer->DrawText(10, y + 20, L"Current mode: " + currentMode, Color::yellow);
}

// 更新物体旋转
void UpdateObjectRotation(Object& object, float angleIncrement = 0.5f, bool rotateX = true, bool rotateY = true, bool rotateZ = false) {
    // 更新全局角度
    g_angle += angleIncrement;
    if (g_angle > 360.0f) {
        g_angle -= 360.0f;
    }
    
    // 获取当前旋转角度
    Vector3f currentRotation = object.transform.rotation;
    
    // 在当前角度的基础上增加旋转
    if (rotateX) {
        currentRotation.x += angleIncrement;
        if (currentRotation.x > 360.0f) currentRotation.x -= 360.0f;
    }
    
    if (rotateY) {
        currentRotation.y += angleIncrement;
        if (currentRotation.y > 360.0f) currentRotation.y -= 360.0f;
    }
    
    if (rotateZ) {
        currentRotation.z += angleIncrement;
        if (currentRotation.z > 360.0f) currentRotation.z -= 360.0f;
    }
    
    // 设置新的旋转角度
    object.transform.SetRotation(currentRotation);
}

// 创建一个立方体网格
Mesh CreateCubeMesh() {
    Mesh mesh;
    
    // 顶点（8个角点）
    Vector4f v0(-0.5f, -0.5f, -0.5f, 1.0f);  // 后下左
    Vector4f v1(0.5f, -0.5f, -0.5f, 1.0f);   // 后下右
    Vector4f v2(0.5f, 0.5f, -0.5f, 1.0f);    // 后上右
    Vector4f v3(-0.5f, 0.5f, -0.5f, 1.0f);   // 后上左
    Vector4f v4(-0.5f, -0.5f, 0.5f, 1.0f);   // 前下左
    Vector4f v5(0.5f, -0.5f, 0.5f, 1.0f);    // 前下右
    Vector4f v6(0.5f, 0.5f, 0.5f, 1.0f);     // 前上右
    Vector4f v7(-0.5f, 0.5f, 0.5f, 1.0f);    // 前上左
    
    // 颜色 - 使对立面颜色互补
    Vector4f red(1.0f, 0.0f, 0.0f, 1.0f);       // 红色（后面）
    Vector4f cyan(0.0f, 1.0f, 1.0f, 1.0f);      // 青色（前面）- 红色的补色
    Vector4f green(0.0f, 1.0f, 0.0f, 1.0f);     // 绿色（左面）
    Vector4f magenta(1.0f, 0.0f, 1.0f, 1.0f);   // 洋红色（右面）- 绿色的补色
    Vector4f blue(0.0f, 0.0f, 1.0f, 1.0f);      // 蓝色（下面）
    Vector4f yellow(1.0f, 1.0f, 0.0f, 1.0f);    // 黄色（上面）- 蓝色的补色
    
    // 法线
    Vector3f nx(1.0f, 0.0f, 0.0f);   // 右
    Vector3f ny(0.0f, 1.0f, 0.0f);   // 上
    Vector3f nz(0.0f, 0.0f, 1.0f);   // 前
    Vector3f nx_neg(-1.0f, 0.0f, 0.0f);  // 左
    Vector3f ny_neg(0.0f, -1.0f, 0.0f);  // 下
    Vector3f nz_neg(0.0f, 0.0f, -1.0f);  // 后
    
    // 纹理坐标
    Vector2f t0(0.0f, 0.0f);
    Vector2f t1(1.0f, 0.0f);
    Vector2f t2(1.0f, 1.0f);
    Vector2f t3(0.0f, 1.0f);

    mesh.Clear();
    
    // 添加顶点（每个面4个顶点，共6个面，总共24个顶点）
    // 后面 (-Z) - 红色
    mesh.AddVertex(Vertex(v0, red, nz_neg, t0)); // 顶点0
    mesh.AddVertex(Vertex(v1, red, nz_neg, t1)); // 顶点1
    mesh.AddVertex(Vertex(v2, red, nz_neg, t2)); // 顶点2
    mesh.AddVertex(Vertex(v3, red, nz_neg, t3)); // 顶点3
    
    // 前面 (+Z) - 青色（红色的补色）
    mesh.AddVertex(Vertex(v4, cyan, nz, t0)); // 顶点4
    mesh.AddVertex(Vertex(v5, cyan, nz, t1)); // 顶点5
    mesh.AddVertex(Vertex(v6, cyan, nz, t2)); // 顶点6
    mesh.AddVertex(Vertex(v7, cyan, nz, t3)); // 顶点7
    
    // 左面 (-X) - 绿色
    mesh.AddVertex(Vertex(v0, green, nx_neg, t0)); // 顶点8
    mesh.AddVertex(Vertex(v3, green, nx_neg, t1)); // 顶点9
    mesh.AddVertex(Vertex(v7, green, nx_neg, t2)); // 顶点10
    mesh.AddVertex(Vertex(v4, green, nx_neg, t3)); // 顶点11
    
    // 右面 (+X) - 洋红色（绿色的补色）
    mesh.AddVertex(Vertex(v1, magenta, nx, t0)); // 顶点12
    mesh.AddVertex(Vertex(v5, magenta, nx, t1)); // 顶点13
    mesh.AddVertex(Vertex(v6, magenta, nx, t2)); // 顶点14
    mesh.AddVertex(Vertex(v2, magenta, nx, t3)); // 顶点15
    
    // 下面 (-Y) - 蓝色
    mesh.AddVertex(Vertex(v0, blue, ny_neg, t0)); // 顶点16
    mesh.AddVertex(Vertex(v4, blue, ny_neg, t1)); // 顶点17
    mesh.AddVertex(Vertex(v5, blue, ny_neg, t2)); // 顶点18
    mesh.AddVertex(Vertex(v1, blue, ny_neg, t3)); // 顶点19
    
    // 上面 (+Y) - 黄色（蓝色的补色）
    mesh.AddVertex(Vertex(v3, yellow, ny, t0)); // 顶点20
    mesh.AddVertex(Vertex(v2, yellow, ny, t1)); // 顶点21
    mesh.AddVertex(Vertex(v6, yellow, ny, t2)); // 顶点22
    mesh.AddVertex(Vertex(v7, yellow, ny, t3)); // 顶点23
    
    // 添加索引（每个面两个三角形，共6个面，总共12个三角形）
    // 从正面看逆时针环绕为正向三角形，正面为顺时针环绕则为背向三角形。（用于背面剔除）
    // 后面
    mesh.AddTriangle(0, 2, 1);
    mesh.AddTriangle(0, 3, 2);
    
    // 前面
    mesh.AddTriangle(4, 5, 6);
    mesh.AddTriangle(4, 6, 7);
    
    // 左面
    mesh.AddTriangle(8, 10, 9);
    mesh.AddTriangle(8, 11, 10);
    
    // 右面
    mesh.AddTriangle(12, 14, 13);
    mesh.AddTriangle(12, 15, 14);
    
    // 下面
    mesh.AddTriangle(16, 18, 17);
    mesh.AddTriangle(16, 19, 18);
    
    // 上面
    mesh.AddTriangle(20, 22, 21);
    mesh.AddTriangle(20, 23, 22);
    
    return mesh;
}

// 绘制2D测试图案
void DrawTestPattern(Window& window)
{
    int width = window.GetWidth();
    int height = window.GetHeight();
    Renderer* renderer = window.GetRenderer();
    
    // 清空背景为黑色
    renderer->ClearBackBuffer(Color::black);
    
    // 绘制坐标轴
    renderer->DrawLine(0, height / 2, width, height / 2, Color::white); // X轴
    renderer->DrawLine(width / 2, 0, width / 2, height, Color::white); // Y轴
    
    // 绘制网格
    // for (int i = 0; i < width; i += 50)
    // {
    //     renderer->DrawLine(i, 0, i, height, RGB(50, 50, 50));
    // }
    
    // for (int i = 0; i < height; i += 50)
    // {
    //     renderer->DrawLine(0, i, width, i, RGB(50, 50, 50));
    // }
    
    // 绘制四边形
    renderer->DrawLine(100, 100, 200, 100, Color::red);
    renderer->DrawLine(200, 100, 200, 200, Color::green);
    renderer->DrawLine(200, 200, 100, 200, Color::blue);
    renderer->DrawLine(100, 200, 100, 100, Color::yellow);
    
    // 绘制五角星
    const int centerX = width - 150;
    const int centerY = height - 150;
    const int radius = 100;
    const int points = 5;
    
    // 计算五角星的顶点
    float angleIncrement = 2.0f * PI / points;
    float startAngle = -PI / 2.0f; // 从顶部开始
    
    int x[points];
    int y[points];
    
    for (int i = 0; i < points; i++)
    {
        float angle = startAngle + i * angleIncrement;
        x[i] = centerX + static_cast<int>(radius * cos(angle));
        y[i] = centerY + static_cast<int>(radius * sin(angle));
    }
    
    // 绘制五角星的连线
    for (int i = 0; i < points; i++)
    {
        int next = (i + 2) % points; // 连接隔一个的点可以画五角星
        renderer->DrawLine(x[i], y[i], x[next], y[next], Color::yellow);
    }
    
    // 显示帧率
    window.DrawFPS();
}

// 加载OBJ模型
bool LoadObjModel(const std::string& filePath) {
    try {
        // 使用ObjFileReader加载模型，应用当前的法线和面片翻转设置
        g_objModel = ObjFileReader::LoadFromFileWithOptions(filePath, g_flipNormals, g_flipFaces);
        
        // 设置模型的变换信息
        g_objModel.transform.SetPosition(Vector3f(0.0f, 0.0f, 0.0f));
        g_objModel.transform.SetScale(Vector3f(1.0f, 1.0f, 1.0f));
        g_objModel.transform.SetRotation(Vector3f(0.0f, 0.0f, 0.0f));
        
        g_objModelLoaded = true;
        std::cout << "OBJ model loaded successfully: " << filePath 
                  << (g_flipNormals ? " (normals flipped)" : "")
                  << (g_flipFaces ? " (faces flipped)" : "")
                  << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load OBJ model: " << e.what() << std::endl;
        g_objModelLoaded = false;
        return false;
    }
}

// 配置纹理设置
void ConfigureTexture(TextureFilterMode filterMode, TextureWrapMode wrapMode, bool generateMipmaps = true) {
    g_texture.SetFilterMode(filterMode);
    g_texture.SetWrapMode(wrapMode);
    
    if (generateMipmaps && filterMode == TextureFilterMode::TRILINEAR) {
        g_texture.GenerateMipmaps();
    }
}

// 加载默认纹理
bool LoadDefaultTexture(Window& window, const std::string& texturePath) {
    bool textureLoaded = g_texture.LoadFromFile(texturePath);
    if (!textureLoaded) {
        // 如果加载失败，使用默认棋盘格纹理
        MessageBox(window.GetHWND(), L"Failed to load texture.\nUsing default checkerboard texture instead.", L"Texture Load Error", MB_OK | MB_ICONWARNING);
        g_texture = Texture::CreateCheckerboard(256, 256, 32, Color::white, Color::black);
    }

    // 设置纹理参数
    ConfigureTexture(TextureFilterMode::TRILINEAR, TextureWrapMode::REPEAT);
    
    // 设置纹理给着色器
    g_textureShader.SetTexture(&g_texture);
    g_texturedBlinnPhongShader.SetTexture(&g_texture);
    
    return textureLoaded;
}

// 加载带纹理的OBJ模型
bool LoadTexturedObjModel(const std::string& objPath, const std::string& texturePath) {
    try {
        // 先加载模型
        if (!LoadObjModel(objPath)) {
            return false;
        }
        
        // 手动加载并关联纹理
        if (!g_texture.LoadFromFile(texturePath)) {
            std::cerr << "无法加载模型贴图: " << texturePath << std::endl;
            // 使用默认棋盘格纹理
            g_texture = Texture::CreateCheckerboard(256, 256, 32, Color::white, Color::black);
        }
        
        // 设置纹理参数
        ConfigureTexture(TextureFilterMode::TRILINEAR, TextureWrapMode::REPEAT);
        
        // 设置纹理给着色器
        g_textureShader.SetTexture(&g_texture);
        g_texturedBlinnPhongShader.SetTexture(&g_texture);
        
        std::cout << "Texture OBJ model loaded successfully: " << objPath 
                  << " Texture: " << texturePath 
                  << (g_flipNormals ? " (normals flipped)" : "")
                  << (g_flipFaces ? " (faces flipped)" : "")
                  << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load textured OBJ model: " << e.what() << std::endl;
        g_objModelLoaded = false;
        return false;
    }
}

// 设置着色器参数
void SetupShaders(float shininess = 32.0f) {
    // 设置着色器高光光泽度
    g_phongShader.SetShininess(shininess);
    g_blinnPhongShader.SetShininess(shininess);
    g_texturedBlinnPhongShader.SetShininess(shininess);
    
    // 更新着色器视点位置
    UpdateShadersViewPosition();
    
    // 更新着色器光照参数
    UpdateLightParams();
}

// 初始化相机
void InitializeCamera(Window& window) {
    float aspect = (float)window.GetWidth() / (float)window.GetHeight();
    g_camera = Camera(
        Vector3f(7.0f, 11.0f, 5.0f),  // 相机位置
        Vector3f(0.0f, 0.0f, 0.0f),  // 视线位置
        Vector3f(0.0f, 1.0f, 0.0f),  // 上方向
        45.0f,                       // 视场角
        aspect,                      // 宽高比
        0.1f,                        // 近平面
        100.0f                       // 远平面
    );
}

// 初始化3D着色器演示
bool InitShaderDemo(Window& window)
{
    Renderer* renderer = window.GetRenderer();

    // 创建立方体
    g_cubeMesh = CreateCubeMesh();
    g_cube = Object(g_cubeMesh, Material(), Transformer());
    
    // 设置立方体位置、缩放和旋转
    g_cube.transform.SetPosition(Vector3f(0.0f, 0.0f, 0.0f));
    g_cube.transform.SetScale(Vector3f(1.0f, 1.0f, 1.0f));
    g_cube.transform.SetRotation(Vector3f(0.0f, 0.0f, 0.0f));

    // 加载带纹理的OBJ模型
    if (LoadTexturedObjModel("D:/VisualStudioProjects/XYHSoftRenderer/TestModel/Cat/cat.obj", "D:/VisualStudioProjects/XYHSoftRenderer/TestModel/Cat/Cat_diffuse.jpg")) {
        g_currentMode = DrawMode::ObjModel3D;
        g_useTextureShaderForObj = true; // 设置为使用带纹理Obj着色器
        std::cout << "Model with texture has been loaded successfully" << std::endl;
        
        // 调整模型的变换
        g_objModel.transform.SetPosition(Vector3f(0.0f, 0.0f, 0.0f));
        g_objModel.transform.SetScale(Vector3f(0.1f, 0.1f, 0.1f)); // 缩小模型以便更好地查看
        g_objModel.transform.SetRotation(Vector3f(270.0f, 0.0f, 0.0f)); // 旋转以便查看
    } else {
        // 如果加载失败，尝试加载茶壶模型作为后备
        if (LoadObjModel("C:\\Users\\Administrator\\Desktop\\teapot.obj")) {
            std::cout << "Utah Teapot OBJ has been loaded as fallback" << std::endl;
        } else {
            std::cerr << "Failed to load any models" << std::endl;
        }
    }
    
    // 初始化相机
    InitializeCamera(window);

    // 设置渲染器的视点位置
    renderer->SetViewPosition(g_camera.position);
    
    // 设置渲染器的变换矩阵
    renderer->SetViewMatrix(g_camera.GetViewMatrix());
    renderer->SetProjectionMatrix(g_camera.GetProjectionMatrix());

    // 设置着色器参数
    SetupShaders(32.0f);
    
    // 如果我们没有在上面加载汽车的纹理，则使用container纹理
    if (!g_useTextureShaderForObj) {
        LoadDefaultTexture(window, "C:\\Users\\Administrator\\Desktop\\container.jpg");
    }

    // 初始化旋转角度
    g_angle = 0.0f;
    
    // 初始化鼠标控制变量
    g_firstMouse = true;
    g_yaw = 0.0f;
    g_pitch = 0.0f;
    g_mouseCaptured = false;

    return true;
}

// 绘制3D着色器演示
void DrawShaderDemo(Window& window)
{
    Renderer* renderer = window.GetRenderer();
    
    // 更新渲染器参数
    UpdateRenderer(renderer);
    
    // 清空缓冲区
    ClearRenderBuffers(renderer, Color(0.2f, 0.2f, 0.2f, 1.0f)); // 深灰色背景
    
    // 更新立方体旋转 - 更好地展示不同面
    UpdateObjectRotation(g_cube);
    
    // 更新光照参数
    UpdateLightParams();
    
    // 使用BlinnPhongShader绘制立方体
    renderer->DrawObject(g_cube, &g_blinnPhongShader);
    
    // 显示场景信息
    std::wstring bgColor = L"background color: (" + 
        std::to_wstring(renderer->GetBackgroundColor().r) + L", " +
        std::to_wstring(renderer->GetBackgroundColor().g) + L", " +
        std::to_wstring(renderer->GetBackgroundColor().b) + L")";
    
    DrawObjectInfo(renderer, g_cube);
    renderer->DrawText(10, 110, bgColor, Color::white);
    DrawCameraInfo(renderer, 130);
    
    // 绘制FPS和操作提示
    //window.DrawFPS();
    DrawStandardControls(renderer, L"3D Filled Demo");
}

// 绘制3D纹理着色器演示
void DrawTextureShaderDemo(Window& window)
{
    Renderer* renderer = window.GetRenderer();
    
    // 更新渲染器参数
    UpdateRenderer(renderer);
    
    // 清空缓冲区
    ClearRenderBuffers(renderer, Color(0.1f, 0.1f, 0.2f, 1.0f)); // 深蓝色背景
    
    // 更新立方体旋转
    UpdateObjectRotation(g_cube);
    
    // 使用纹理着色器绘制立方体
    renderer->DrawObject(g_cube, &g_textureShader);
    
    // 显示场景信息
    std::wstring bgColor = L"background color: (" + 
        std::to_wstring(renderer->GetBackgroundColor().r) + L", " +
        std::to_wstring(renderer->GetBackgroundColor().g) + L", " +
        std::to_wstring(renderer->GetBackgroundColor().b) + L")";
    
    DrawObjectInfo(renderer, g_cube);
    renderer->DrawText(10, 130, bgColor, Color::white);
    DrawCameraInfo(renderer, 150);
    DrawTextureInfo(renderer, g_texture, 170, L"container.jpg");
    
    // 绘制FPS和操作提示
    //window.DrawFPS();
    DrawStandardControls(renderer, L"3D Texture Demo");
    renderer->DrawText(10, 50, L"F: Switch filtering mode, T: Switch wrapping mode, 1: Checkerboard texture, 2: Gradient texture, 3: wall.bmp texture", Color::white);
}

// 绘制3D纹理Blinn-Phong着色器演示
void DrawTextureBlinnPhongShaderDemo(Window& window)
{
    Renderer* renderer = window.GetRenderer();
    
    // 更新渲染器参数
    UpdateRenderer(renderer);
    
    // 清空缓冲区
    ClearRenderBuffers(renderer, Color(0.1f, 0.1f, 0.2f, 1.0f)); // 深蓝色背景
    
    // 更新立方体旋转
    UpdateObjectRotation(g_cube);

    // 更新光照参数
    UpdateLightParams();
    
    // 使用纹理Blinn-Phong着色器绘制立方体
    renderer->DrawObject(g_cube, &g_texturedBlinnPhongShader);
    
    // 显示场景信息
    std::wstring bgColor = L"background color: (" + 
        std::to_wstring(renderer->GetBackgroundColor().r) + L", " +
        std::to_wstring(renderer->GetBackgroundColor().g) + L", " +
        std::to_wstring(renderer->GetBackgroundColor().b) + L")";
    
    DrawObjectInfo(renderer, g_cube);
    renderer->DrawText(10, 130, bgColor, Color::white);
    DrawCameraInfo(renderer, 150);
    DrawTextureInfo(renderer, g_texture, 170, L"container.jpg");
    
    // 绘制FPS和操作提示
    //window.DrawFPS();
    DrawStandardControls(renderer, L"3D Textured Blinn-Phong Demo");
    renderer->DrawText(10, 50, L"F: Switch filtering mode, T: Switch wrapping mode, 1: Checkerboard texture, 2: Gradient texture, 3: wall.bmp texture", Color::white);
}

// 绘制OBJ模型
void DrawObjModel(Window& window) {
    Renderer* renderer = window.GetRenderer();
    
    // 设置背面剔除状态
    renderer->SetCullMode(g_cullMode);
    
    // 更新渲染器参数
    UpdateRenderer(renderer);
    
    // 清空缓冲区
    ClearRenderBuffers(renderer, Color(0.05f, 0.05f, 0.1f, 1.0f)); // 深蓝色背景
    
    // 检查OBJ模型是否已加载
    if (!g_objModelLoaded) {
        renderer->DrawText(10, 70, L"No OBJ model loaded. Press L to load a model.", Color::white);
        window.DrawFPS();
        return;
    }
    
    // 更新模型旋转
    UpdateObjectRotation(g_objModel, 0.5f, false, true, false);

    // 更新光照参数
    // UpdateLightParams(0.2f, 0.7f);
    UpdateLightParams();
    
    // 更新着色器的视点位置
    UpdateShadersViewPosition();
    
    // 根据全局变量决定使用哪个着色器
    if (g_useTextureShaderForObj) {
        // 使用带纹理的Blinn-Phong着色器
        renderer->DrawObject(g_objModel, &g_texturedBlinnPhongShader);
        renderer->DrawText(10, 190, L"Use textured Blinn-Phong shader (press M to switch)", Color::white);
    } else {
        // 使用普通Blinn-Phong着色器
        // renderer->DrawObject(g_objModel, &g_blinnPhongShader);
        renderer->DrawObject(g_objModel, &g_textureShader); // 使用普通纹理着色器
        renderer->DrawText(10, 190, L"Use normal Blinn-Phong shader (press M to switch)", Color::white);
    }
    
    // 显示模型信息
    std::wstring meshInfo = L"Vertices: " + std::to_wstring(g_objModel.mesh.vertices.size()) + 
                           L", Triangles: " + std::to_wstring(g_objModel.mesh.indices.size());
    
    DrawObjectInfo(renderer, g_objModel);
    DrawCameraInfo(renderer, 150);
    DrawTextureInfo(renderer, g_texture, 170);
    
    renderer->DrawText(10, 50, meshInfo, Color::white);

    Renderer::CullMode currentMode = renderer->GetCullMode();

    std::wstring meshOptions = L"Mesh Options: " + 
        std::wstring(g_flipNormals ? L"Normals Flipped, " : L"") + 
        std::wstring(g_flipFaces ? L"Faces Flipped, " : L"") +
        std::wstring(currentMode == Renderer::CullMode::CULL_BACK ? L"Backface Culling ON" : 
                     currentMode == Renderer::CullMode::CULL_FRONT ? L"Frontface Culling ON" : 
                     L"No Culling");
    
    renderer->DrawText(10, 210, meshOptions, Color::white);
    
    // 绘制FPS和操作提示
    //window.DrawFPS();
    DrawStandardControls(renderer, L"3D OBJ Model Demo");
    renderer->DrawText(10, 30, L"L: Load model, P: Load model with texture, M: Switch shader", Color::white);
    renderer->DrawText(10, 230, L"N: Flip normals, V: Flip faces, B: Toggle backface culling", Color::white);
}

// 纹理相关操作函数
// 切换纹理过滤模式
void SwitchFilterMode(HWND hwnd) {
    if (g_texture.filterMode == TextureFilterMode::NEAREST) {
        g_texture.SetFilterMode(TextureFilterMode::BILINEAR);
        MessageBox(hwnd, L"Switched to bilinear filtering", L"Filter Mode", MB_OK);
    } 
    else if (g_texture.filterMode == TextureFilterMode::BILINEAR) {
        g_texture.SetFilterMode(TextureFilterMode::TRILINEAR);
        g_texture.GenerateMipmaps();
        MessageBox(hwnd, L"Switched to trilinear filtering with mipmaps", L"Filter Mode", MB_OK);
    }
    else {
        g_texture.SetFilterMode(TextureFilterMode::NEAREST);
        MessageBox(hwnd, L"Switched to nearest filtering", L"Filter Mode", MB_OK);
    }
}

// 切换纹理环绕模式
void SwitchWrapMode(HWND hwnd) {
    if (g_texture.wrapMode == TextureWrapMode::REPEAT) {
        g_texture.SetWrapMode(TextureWrapMode::CLAMP);
        MessageBox(hwnd, L"Already switched to clamp wrapping mode", L"Wrapping Mode", MB_OK | MB_ICONINFORMATION);
    } else if (g_texture.wrapMode == TextureWrapMode::CLAMP) {
        g_texture.SetWrapMode(TextureWrapMode::MIRROR);
        MessageBox(hwnd, L"Already switched to mirror wrapping mode", L"Wrapping Mode", MB_OK | MB_ICONINFORMATION);
    } else {
        g_texture.SetWrapMode(TextureWrapMode::REPEAT);
        MessageBox(hwnd, L"Already switched to repeat wrapping mode", L"Wrapping Mode", MB_OK | MB_ICONINFORMATION);
    }
}

// 加载棋盘格纹理
void LoadCheckerboardTexture(HWND hwnd) {
    g_texture = Texture::CreateCheckerboard(256, 256, 32, Color::white, Color::black);
    g_texture.SetFilterMode(g_texture.filterMode); // 保持当前过滤模式
    if (g_texture.filterMode == TextureFilterMode::TRILINEAR) {
        g_texture.GenerateMipmaps();
    }
    g_textureShader.SetTexture(&g_texture);
    g_texturedBlinnPhongShader.SetTexture(&g_texture);
    MessageBox(hwnd, L"Already switched to checkerboard texture", L"Texture Switch", MB_OK | MB_ICONINFORMATION);
}

// 加载渐变纹理
void LoadGradientTexture(HWND hwnd) {
    g_texture = Texture::CreateGradient(256, 256, Color::red, Color::blue, true);
    g_texture.SetFilterMode(TextureFilterMode::BILINEAR);
    g_texture.SetWrapMode(TextureWrapMode::REPEAT);
    g_textureShader.SetTexture(&g_texture);
    g_texturedBlinnPhongShader.SetTexture(&g_texture);
    MessageBox(hwnd, L"Already switched to gradient texture", L"Texture Switch", MB_OK | MB_ICONINFORMATION);
}

// 加载指定图像纹理
void LoadImageTexture(HWND hwnd, const std::string& texturePath) {
    bool success = g_texture.LoadFromFile(texturePath);
    g_texture.SetFilterMode(TextureFilterMode::BILINEAR);
    g_texture.SetWrapMode(TextureWrapMode::REPEAT);
    g_textureShader.SetTexture(&g_texture);
    g_texturedBlinnPhongShader.SetTexture(&g_texture);
    
    if (success) {
        MessageBox(hwnd, L"Texture loaded successfully", L"Texture Switch", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBox(hwnd, L"Failed to load texture", L"Texture Load Error", MB_OK | MB_ICONERROR);
    }
}

// 打开文件对话框加载OBJ模型
bool OpenFileDialogAndLoadObjModel(HWND hwnd) {
    OPENFILENAME ofn;
    wchar_t szFile[260] = {0};
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"OBJ Files (*.obj)\0*.obj\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileName(&ofn)) {
        // 转换为std::string
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, szFile, -1, NULL, 0, NULL, NULL);
        std::string filePath(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, szFile, -1, &filePath[0], size_needed, NULL, NULL);
        filePath.resize(size_needed - 1);  // 去掉结尾的空字符
        
        // 加载模型
        if (LoadObjModel(filePath)) {
            g_currentMode = DrawMode::ObjModel3D;
            MessageBox(hwnd, L"OBJ model loaded successfully!", L"Success", MB_OK | MB_ICONINFORMATION);
            return true;
        } else {
            MessageBox(hwnd, L"Failed to load OBJ model!", L"Error", MB_OK | MB_ICONERROR);
            return false;
        }
    }
    return false;
}

// 打开文件对话框加载带纹理的OBJ模型
bool OpenFileDialogAndLoadTexturedObjModel(HWND hwnd) {
    // 打开文件对话框选择OBJ文件
    OPENFILENAME ofn;
    wchar_t szObjFile[260] = {0};
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szObjFile;
    ofn.nMaxFile = sizeof(szObjFile);
    ofn.lpstrFilter = L"OBJ Files (*.obj)\0*.obj\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileName(&ofn)) {
        // 打开文件对话框选择纹理文件
        wchar_t szTextureFile[260] = {0};
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = szTextureFile;
        ofn.nMaxFile = sizeof(szTextureFile);
        ofn.lpstrFilter = L"Image Files (*.jpg;*.jpeg;*.png;*.bmp)\0*.jpg;*.jpeg;*.png;*.bmp\0All Files (*.*)\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        
        if (GetOpenFileName(&ofn)) {
            // 转换OBJ路径为std::string
            int obj_size_needed = WideCharToMultiByte(CP_UTF8, 0, szObjFile, -1, NULL, 0, NULL, NULL);
            std::string objPath(obj_size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, szObjFile, -1, &objPath[0], obj_size_needed, NULL, NULL);
            objPath.resize(obj_size_needed - 1);  // 去掉结尾的空字符
            
            // 转换纹理路径为std::string
            int tex_size_needed = WideCharToMultiByte(CP_UTF8, 0, szTextureFile, -1, NULL, 0, NULL, NULL);
            std::string texturePath(tex_size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, szTextureFile, -1, &texturePath[0], tex_size_needed, NULL, NULL);
            texturePath.resize(tex_size_needed - 1);  // 去掉结尾的空字符
            
            // 加载带纹理的模型
            if (LoadTexturedObjModel(objPath, texturePath)) {
                g_currentMode = DrawMode::ObjModel3D;
                g_useTextureShaderForObj = true;  // 自动切换到纹理着色器
                MessageBox(hwnd, L"Texture OBJ model loaded successfully!", L"Success", MB_OK | MB_ICONINFORMATION);
                return true;
            } else {
                MessageBox(hwnd, L"Failed to load textured OBJ model!", L"Error", MB_OK | MB_ICONERROR);
                return false;
            }
        }
    }
    return false;
}

// 切换绘制模式
void SwitchDrawMode() {
    if (g_currentMode == DrawMode::Test2D)
        g_currentMode = DrawMode::Shader3D;
    else if (g_currentMode == DrawMode::Shader3D)
        g_currentMode = DrawMode::TextureShader3D;
    else if (g_currentMode == DrawMode::TextureShader3D)
        g_currentMode = DrawMode::TextureBlinnPhong3D;
    else if (g_currentMode == DrawMode::TextureBlinnPhong3D)
        g_currentMode = DrawMode::ObjModel3D;
    else
        g_currentMode = DrawMode::Test2D;
}

// 鼠标移动更新相机方向
void UpdateCameraDirectionWithMouse(float xpos, float ypos) {
    // 第一次移动时初始化lastX和lastY
    if (g_firstMouse)
    {
        g_lastX = xpos;
        g_lastY = ypos;
        g_firstMouse = false;
        return;
    }
    
    // 计算鼠标移动的偏移量
    float xoffset = xpos - g_lastX;
    float yoffset = g_lastY - ypos; // 注意这里是相反的，因为y坐标是从底部往顶部依次增大的
    g_lastX = xpos;
    g_lastY = ypos;
    
    // 应用灵敏度
    xoffset *= g_mouseSensitivity;
    yoffset *= g_mouseSensitivity;
    
    // 更新欧拉角
    g_yaw += xoffset;
    g_pitch += yoffset;
    
    // 限制俯仰角，防止万向节锁
    if (g_pitch > 89.0f)
        g_pitch = 89.0f;
    if (g_pitch < -89.0f)
        g_pitch = -89.0f;
    
    // 计算新的相机方向
    Vector3f front;
    front.x = cosf(toRadians(g_yaw)) * cosf(toRadians(g_pitch));
    front.y = sinf(toRadians(g_pitch));
    front.z = sinf(toRadians(g_yaw)) * cosf(toRadians(g_pitch));
    front = front.normalize();
    
    // 更新相机目标点
    g_camera.target = g_camera.position + front;
    g_camera.UpdateViewMatrix();
}

// 处理按键输入
void HandleKeyDown(HWND hwnd, WPARAM wParam) {
    // 相机移动速度
    const float cameraSpeed = 0.2f;
    // 光照强度调整速度
    const float lightIntensitySpeed = 0.1f;
    
    // 空格键：切换绘制模式
    if (wParam == VK_SPACE) {
        SwitchDrawMode();
    }
    // L键：加载OBJ模型
    else if (wParam == 'L' || wParam == 'l') {
        OpenFileDialogAndLoadObjModel(hwnd);
    }
    // P键：加载带纹理的OBJ模型
    else if (wParam == 'P' || wParam == 'p') {
        OpenFileDialogAndLoadTexturedObjModel(hwnd);
    }
    // M键：切换模型渲染模式（是否使用纹理）
    else if (wParam == 'M' || wParam == 'm') {
        g_useTextureShaderForObj = !g_useTextureShaderForObj;
        
        if (g_useTextureShaderForObj) {
            MessageBox(hwnd, L"Has switched to textured rendering mode", L"Rendering Mode", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBox(hwnd, L"Has switched to normal rendering mode", L"Rendering Mode", MB_OK | MB_ICONINFORMATION);
        }
    }
    // +/-键：缩放OBJ模型
    else if (wParam == VK_ADD || wParam == VK_OEM_PLUS) {
        if (g_objModelLoaded) {
            Vector3f scale = g_objModel.transform.scale;
            scale = scale * 1.1f;  // 放大10%
            g_objModel.transform.SetScale(scale);
        }
    }
    else if (wParam == VK_SUBTRACT || wParam == VK_OEM_MINUS) {
        if (g_objModelLoaded) {
            Vector3f scale = g_objModel.transform.scale;
            scale = scale * 0.9f;  // 缩小10%
            g_objModel.transform.SetScale(scale);
        }
    }
    // WASD：移动相机
    if (wParam == 'W' || wParam == 'w') {
        g_camera.MoveForward(cameraSpeed);
    }
    if (wParam == 'S' || wParam == 's') {
        g_camera.MoveForward(-cameraSpeed);
    }
    if (wParam == 'A' || wParam == 'a') {
        g_camera.MoveRight(-cameraSpeed);
    }
    if (wParam == 'D' || wParam == 'd') {
        g_camera.MoveRight(cameraSpeed);
    }
    // 上下箭头：上下移动相机
    if (wParam == VK_UP) {
        g_camera.MoveUp(cameraSpeed);
    }
    if (wParam == VK_DOWN) {
        g_camera.MoveUp(-cameraSpeed);
    }
    // 左右箭头：调整光照强度
    else if (wParam == VK_LEFT) {
        g_lightIntensity = (std::max)(0.1f, g_lightIntensity - lightIntensitySpeed);
        UpdateLightParams();
    }
    else if (wParam == VK_RIGHT) {
        g_lightIntensity = (std::min)(2.0f, g_lightIntensity + lightIntensitySpeed);
        UpdateLightParams();
    }
    // F键：调整纹理过滤模式
    else if (wParam == 'F' || wParam == 'f') {
        SwitchFilterMode(hwnd);
    }
    // T键：调整纹理环绕模式
    else if (wParam == 'T' || wParam == 't') {
        SwitchWrapMode(hwnd);
    }
    // 1,2,3键：切换纹理
    else if (wParam == '1') {
        LoadCheckerboardTexture(hwnd);
    }
    else if (wParam == '2') {
        LoadGradientTexture(hwnd);
    }
    else if (wParam == '3') {
        LoadImageTexture(hwnd, "C:\\Users\\Administrator\\Desktop\\wall.bmp");
    }
    // N键：切换法线翻转
    else if (wParam == 'N' || wParam == 'n') {
        g_flipNormals = !g_flipNormals;
        
        if (g_objModelLoaded) {
            std::wstring message = g_flipNormals ? 
                L"Normals will be flipped when you load models. Please reload your model." : 
                L"Normals will be used as-is when you load models. Please reload your model.";
            MessageBox(hwnd, message.c_str(), L"Normal Flip Setting", MB_OK | MB_ICONINFORMATION);
        }
    }
    // V键：切换面片翻转
    else if (wParam == 'V' || wParam == 'v') {
        g_flipFaces = !g_flipFaces;
        
        if (g_objModelLoaded) {
            std::wstring message = g_flipFaces ? 
                L"Face winding order will be flipped when you load models. Please reload your model." : 
                L"Face winding order will be used as-is when you load models. Please reload your model.";
            MessageBox(hwnd, message.c_str(), L"Face Flip Setting", MB_OK | MB_ICONINFORMATION);
        }
    }
    // B键：切换背面剔除
    else if (wParam == 'B' || wParam == 'b') {
        // 获取渲染器
        HWND hwnd_from_handle = hwnd;
        Window* windowPtr = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd_from_handle, GWLP_USERDATA));
        
        if (windowPtr) {
            Renderer* renderer = windowPtr->GetRenderer();
            
            // 获取当前的剔除模式
            Renderer::CullMode currentMode = renderer->GetCullMode();
            
            // 循环切换三种模式
            switch (currentMode) {
                case Renderer::CullMode::CULL_BACK:
                    renderer->SetCullMode(Renderer::CullMode::CULL_FRONT); // 剔除正面
                    MessageBox(hwnd, L"Has switched to front face culling mode (only render back face)", L"Culling Mode", MB_OK | MB_ICONINFORMATION);
                    break;
                case Renderer::CullMode::CULL_FRONT:
                    renderer->SetCullMode(Renderer::CullMode::CULL_NONE); // 双面绘制
                    MessageBox(hwnd, L"Has switched to double-sided rendering mode (no face culling)", L"Culling Mode", MB_OK | MB_ICONINFORMATION);
                    break;
                case Renderer::CullMode::CULL_NONE:
                    renderer->SetCullMode(Renderer::CullMode::CULL_BACK); // 剔除背面
                    MessageBox(hwnd, L"Has switched to back face culling mode (only render back face)", L"Culling Mode", MB_OK | MB_ICONINFORMATION);
                    break;
            }
            
            // 更新全局变量以保持一致
            g_cullMode = renderer->GetCullMode();
        }
    }
}

// 窗口消息处理函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_KEYDOWN:
        HandleKeyDown(hwnd, wParam);
        break;
    case WM_LBUTTONDOWN:
        {
            // 鼠标左键按下时捕获鼠标
            SetCapture(hwnd);
            g_mouseCaptured = true;
            
            // 获取鼠标位置并初始化
            g_lastX = (float)GET_X_LPARAM(lParam);
            g_lastY = (float)GET_Y_LPARAM(lParam);
            g_firstMouse = false;
        }
        break;
    case WM_LBUTTONUP:
        {
            // 鼠标左键释放时取消捕获
            ReleaseCapture();
            g_mouseCaptured = false;
        }
        break;
    case WM_MOUSEMOVE:
        {
            // 只有在鼠标被捕获时才处理移动
            if (g_mouseCaptured)
            {
                float xpos = (float)GET_X_LPARAM(lParam);
                float ypos = (float)GET_Y_LPARAM(lParam);
                UpdateCameraDirectionWithMouse(xpos, ypos);
            }
        }
        break;
    case WM_CLOSE:
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// 主函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 创建800x600窗口
    Window window(800, 600, L"XYH Soft Renderer");
    
    if (!window.Initialize())
    {
        MessageBox(nullptr, L"Window initialization failed!", L"Error", MB_OK);
        return -1;
    }
    
    // 设置窗口消息处理函数
    SetWindowLongPtr(window.GetHWND(), GWLP_WNDPROC, (LONG_PTR)WindowProc);
    
    // 获取渲染器
    Renderer* renderer = window.GetRenderer();
    
    // 设置背景颜色
    renderer->SetBackgroundColor(Color::black); // 设置背景颜色
    
    // 初始化着色器演示
    if (!InitShaderDemo(window)) 
    {
        MessageBox(nullptr, L"Shader initialization failed!", L"Error", MB_OK);
        return -1;
    }
    
    // 获取窗口DC
    HDC hdc = GetDC(window.GetHWND());
    
    // 主循环
    bool running = true;
    while (running)
    {
        // 处理窗口消息
        if (!window.ProcessMessages())
        {
            running = false;
            break;
        }
        
        // 根据当前模式绘制
        RenderCurrentScene(window);
        
        // 交换缓冲区并显示
        renderer->SwapBuffers(hdc);
        
        // 更新帧率
        window.UpdateFPS();
    }
    
    // 释放DC
    ReleaseDC(window.GetHWND(), hdc);
    
    return 0;
}

// 根据当前模式调用相应的绘制函数
void RenderCurrentScene(Window& window)
{
    switch (g_currentMode)
    {
    case DrawMode::Test2D:
        DrawTestPattern(window);
        break;
    case DrawMode::Shader3D:
        DrawShaderDemo(window);
        break;
    case DrawMode::TextureShader3D:
        DrawTextureShaderDemo(window);
        break;
    case DrawMode::TextureBlinnPhong3D:
        DrawTextureBlinnPhongShaderDemo(window);
        break;
    case DrawMode::ObjModel3D:
        DrawObjModel(window);
        break;
    }
}
