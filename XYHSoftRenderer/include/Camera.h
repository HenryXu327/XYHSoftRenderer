#pragma once

#include "Vector.h"
#include "Matrix.h"

class Camera {
public:
    // 相机参数
    Vector3f position;        // 相机位置
    Vector3f target;          // 目标位置
    Vector3f up;              // 上方向
    
    // 透视投影参数
    float fov;                // 视场角（弧度）
    float aspect;             // 宽高比
    float nearZ;              // 近平面距离
    float farZ;               // 远平面距离
    
    // 矩阵
    Matrix viewMatrix;        // 视图矩阵
    Matrix projMatrix;        // 投影矩阵
    
public:
    // 构造函数
    Camera();
    Camera(
        const Vector3f& position, 
        const Vector3f& target, 
        const Vector3f& up = Vector3f(0.0f, 1.0f, 0.0f), 
        float fov = 45.0f, 
        float aspect = 4.0f / 3.0f, 
        float nearZ = 0.1f, 
        float farZ = 100.0f
    );
    
    // 设置相机位置和方向
    void SetPosition(const Vector3f& pos);
    void SetTarget(const Vector3f& target);
    void SetUpDirection(const Vector3f& up);
    
    // 设置透视投影参数
    void SetPerspective(float fov, float aspect, float nearZ, float farZ);
    
    // 更新矩阵
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();
    
    // 获取矩阵
    Matrix GetViewMatrix() const;
    Matrix GetProjectionMatrix() const;
    
    // 相机移动方法
    void MoveForward(float distance);
    void MoveRight(float distance);
    void MoveUp(float distance);
    
    // 相机旋转方法
    void Rotate(float yaw, float pitch);
}; 