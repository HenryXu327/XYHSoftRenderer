#pragma once

#include <Windows.h>
#include <string>
#include "Renderer.h"

class Window {
public:
    Window(int width, int height, const std::wstring& title);
    ~Window();

    bool Initialize();
    void Shutdown();
    bool ProcessMessages();
    
    // 帧率相关
    void UpdateFPS();
    void DrawFPS();
    float GetFPS() const { return m_fps; }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    HWND GetHWND() const { return m_hWnd; }
    
    // 获取渲染器
    Renderer* GetRenderer() { return m_renderer; }

private:
    // 窗口尺寸
    int m_width;
    int m_height;
    std::wstring m_title;

    // Windows相关
    HWND m_hWnd;
    HINSTANCE m_hInstance;
    HDC m_hdc;
    
    // 渲染器
    Renderer* m_renderer;
    
    // 帧率计算相关
    LARGE_INTEGER m_frequency;
    LARGE_INTEGER m_lastTime;
    int m_frameCount;
    float m_fps;
    float m_fpsUpdateInterval;
    
    // 窗口过程函数
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}; 