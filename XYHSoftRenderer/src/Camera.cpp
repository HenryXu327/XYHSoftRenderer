#include "../include/Camera.h"
#include "../include/MyMath.h"

Camera::Camera()
    : position(0.0f, 0.0f, 5.0f),
      target(0.0f, 0.0f, 0.0f),
      up(0.0f, 1.0f, 0.0f),
      fov(toRadians(45.0f)),
      aspect(4.0f / 3.0f),
      nearZ(0.1f),
      farZ(100.0f)
{
    UpdateViewMatrix();
    UpdateProjectionMatrix();
}

Camera::Camera(const Vector3f& position, const Vector3f& target, const Vector3f& up, float fov, float aspect, float nearZ, float farZ)
    : position(position),
      target(target),
      up(up),
      fov(toRadians(fov)),  // 将度转换为弧度
      aspect(aspect),
      nearZ(nearZ),
      farZ(farZ)
{
    UpdateViewMatrix();
    UpdateProjectionMatrix();
}

void Camera::SetPosition(const Vector3f& pos)
{
    position = pos;
    UpdateViewMatrix();
}

void Camera::SetTarget(const Vector3f& tgt)
{
    target = tgt;
    UpdateViewMatrix();
}

void Camera::SetUpDirection(const Vector3f& upDir)
{
    up = upDir;
    UpdateViewMatrix();
}

void Camera::SetPerspective(float fieldOfView, float aspectRatio, float near, float far)
{
    fov = toRadians(fieldOfView);  // 将度转换为弧度
    aspect = aspectRatio;
    nearZ = near;
    farZ = far;
    UpdateProjectionMatrix();
}

void Camera::UpdateViewMatrix()
{
    viewMatrix = Matrix::lookAt(position, target, up);
}

void Camera::UpdateProjectionMatrix()
{
    projMatrix = Matrix::perspective(fov, aspect, nearZ, farZ);
}

Matrix Camera::GetViewMatrix() const
{
    return viewMatrix;
}

Matrix Camera::GetProjectionMatrix() const
{
    return projMatrix;
}

void Camera::MoveForward(float distance)
{
    // 计算前方向（目标点到相机的反方向）
    Vector3f forward = (target - position).normalize();
    position = position + forward * distance;
    target = target + forward * distance;
    UpdateViewMatrix();
}

void Camera::MoveRight(float distance)
{
    // 计算右方向（前方向和上方向的叉积）
    Vector3f forward = (target - position).normalize();
    Vector3f right = Vector3f::cross(forward, up).normalize();
    position = position + right * distance;
    target = target + right * distance;
    UpdateViewMatrix();
}

void Camera::MoveUp(float distance)
{
    // 直接使用上方向移动
    position = position + up * distance;
    target = target + up * distance;
    UpdateViewMatrix();
}

void Camera::Rotate(float yaw, float pitch)
{
    // 计算相机到目标的向量
    Vector3f direction = (target - position).normalize();
    
    // 将方向转换为球坐标
    float length = (target - position).magnitude();
    float theta = atan2f(direction.z, direction.x);  // 方位角
    float phi = acosf(direction.y);                  // 极角
    
    // 应用旋转
    theta -= toRadians(yaw);    // 左右旋转
    phi -= toRadians(pitch);    // 上下旋转
    
    // 限制极角范围，防止万向节锁
    phi = std::max(0.1f, std::min(phi, PI - 0.1f));
    
    // 将球坐标转换回笛卡尔坐标
    direction.x = sinf(phi) * cosf(theta);
    direction.y = cosf(phi);
    direction.z = sinf(phi) * sinf(theta);
    
    // 更新目标点
    target = position + direction * length;
    
    UpdateViewMatrix();
} 