# XYHSoftRenderer
A pure software rasterization renderer for the Windows platform that does not rely on third-party graphics libraries such as Direct3D or OpenGL.

- 项目目标：实现一个纯软件的、不依赖Direct3D或OpenGL等第三方图形库的光栅化渲染器
- 开发环境：Windows平台，除Windows.h、WindowsX.h、gdiplus.h无其他外部库，仅使用WindowsGDI绘制像素点功能，全C++实现
- 基本功能：支持3D模型渲染、纹理映射（包括生成Mipmap三线性插值）、经验模型光照等

![纹理](D:\Typora\assets\纹理-1744883016374-1.png)

![光照](D:\Typora\assets\光照-1744883016374-2.png)

![纹理+光照](D:\Typora\assets\纹理+光照-1744883016374-3.png)

<video src=".\展示结果/演示视频.mp4"></video>

## 
