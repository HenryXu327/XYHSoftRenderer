#include "../include/Window.h"
#include <sstream>
#include "../include/Color.h"

Window::Window(int width, int height, const std::wstring& title)
    : m_width(width), m_height(height), m_title(title),
    m_hWnd(nullptr), m_hInstance(GetModuleHandle(nullptr)),
    m_hdc(nullptr), m_renderer(nullptr),
    m_frameCount(0), m_fps(0.0f), m_fpsUpdateInterval(0.5f)
{
    // 初始化性能计数器频率
    QueryPerformanceFrequency(&m_frequency);
    QueryPerformanceCounter(&m_lastTime);
}

Window::~Window()
{
    Shutdown();
}

bool Window::Initialize()
{
    // 注册窗口类
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"XYHSoftRendererClass";

    if (!RegisterClassEx(&wc))
        return false;

    // 创建窗口
    RECT windowRect = { 0, 0, m_width, m_height };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
    
    m_hWnd = CreateWindow(
        wc.lpszClassName, m_title.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
        nullptr, nullptr, m_hInstance, nullptr
    );

    if (!m_hWnd)
        return false;

    // 显示窗口
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    // 将Window对象指针存储在窗口属性中
    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // 获取DC
    m_hdc = GetDC(m_hWnd);
    
    // 创建渲染器
    m_renderer = new Renderer(m_width, m_height);
    if (!m_renderer->Initialize(m_hdc))
        return false;

    return true;
}

void Window::Shutdown()
{
    if (m_renderer)
    {
        m_renderer->Shutdown();
        delete m_renderer;
        m_renderer = nullptr;
    }

    if (m_hdc && m_hWnd)
    {
        ReleaseDC(m_hWnd, m_hdc);
        m_hdc = nullptr;
    }

    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}

bool Window::ProcessMessages()
{
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return false;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

void Window::UpdateFPS()
{
    // 增加帧计数
    m_frameCount++;
    
    // 获取当前时间
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    
    // 计算时间差（秒）
    double timeElapsed = static_cast<double>(currentTime.QuadPart - m_lastTime.QuadPart) / 
                         static_cast<double>(m_frequency.QuadPart);
    
    // 如果经过了足够的时间，更新FPS
    if (timeElapsed >= m_fpsUpdateInterval)
    {
        // 计算FPS
        m_fps = static_cast<float>(m_frameCount) / static_cast<float>(timeElapsed);
        
        // 重置计数器
        m_frameCount = 0;
        m_lastTime = currentTime;
        
        // 更新窗口标题以显示FPS
        std::wstringstream ss;
        ss << m_title << L" - FPS: " << static_cast<int>(m_fps);
        SetWindowText(m_hWnd, ss.str().c_str());
    }
}

void Window::DrawFPS()
{
    // 创建显示FPS的字符串
    std::wstringstream ss;
    ss << L"FPS: " << static_cast<int>(m_fps);
    
    // 在屏幕左上角绘制FPS，使用黄色
    m_renderer->DrawText(10, 10, ss.str(), Color::yellow);
}

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
} 