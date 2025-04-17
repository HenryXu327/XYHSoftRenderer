#pragma once
#include <algorithm>
#include "Vector.h"
#include "Matrix.h"

#define PI 3.1415926535f

// 角度转弧度
static inline float toRadians(float degrees)
{
    return degrees * (PI / 180.0f);
}

// 弧度转角度
static inline float toDegrees(float radians)
{
    return radians * (180.0f / PI);
}

// 计算两个向量的夹角(弧度)
static float radiansAngleBetween(const Vector3f& v1, const Vector3f& v2)
{
    float radians = acosf(v1.dot(v2) / (v1.magnitude() * v2.magnitude())); 
    return radians;
}

// 计算两个向量的夹角(角度)
static float degreesAngleBetween(const Vector3f& v1, const Vector3f& v2)
{
    float radians = acosf(v1.dot(v2) / (v1.magnitude() * v2.magnitude())); 
    return toDegrees(radians);
}

// 夹逼
inline static float clamp(float value, float _min, float _max)
{
    if (value < _min)
        return _min;
    else if (value > _max)
        return _max;
    else
        return value;
}

// 夹逼0-1
inline static float clamp01(float value)
{
    return clamp(value, 0.0f, 1.0f);
}

// 线性插值
static float lerp(float start, float end, float weight)
{
    return start + (end - start) * weight;
}


