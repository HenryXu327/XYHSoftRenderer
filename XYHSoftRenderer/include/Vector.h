#pragma once
#include <iostream>
#include <cassert>
#include <cmath>
#include <algorithm>

// 在文件前部添加常量
#define EPSILON 0.000001f // 添加一个小的阈值常量用于浮点数比较

// 二维向量
template <typename T>
class Vector2
{
public:
    T x, y;

    Vector2() : x(0), y(0) {}
    Vector2(T x, T y) : x(x), y(y) {}
    Vector2(const Vector2<T>& vec) : x(vec.x), y(vec.y) {}

    ~Vector2() {}

public:
    Vector2<T> operator+(const Vector2<T>& vec) const;
    Vector2<T> operator-(const Vector2<T>& vec) const;
    Vector2<T> operator-() const;  // 单目减运算符
    Vector2<T> operator*(const float& scalar) const;
    Vector2<T> operator/(const float& scalar) const;

    // 复合赋值运算符
    Vector2<T>& operator+=(const Vector2<T>& vec);
    Vector2<T>& operator-=(const Vector2<T>& vec);
    Vector2<T>& operator*=(const float& scalar);
    Vector2<T>& operator/=(const float& scalar);
    Vector2<T>& operator*=(const Vector2<T>& vec);  // 向量分量相乘

    // 向量模长的平方
    float magnitudeSquared() const;

    // 向量模长
    float magnitude() const;

    // 向量归一化
    Vector2<T>& normalize();

    // 向量点乘
    float dot(const Vector2<T>& vec) const;
    static float dot(const Vector2<T>& vec1, const Vector2<T>& vec2);

    // 向量插值
    static Vector2<T> lerp(const Vector2<T>& vec1, const Vector2<T>& vec2, float weight);

    // 标准化
    static Vector2<T> standardization(const Vector2<T>& vec);

    // 向量反射
    static Vector2<T> reflect(const Vector2<T>& incident, const Vector2<T>& normal);

    // 零向量
    static const Vector2<T> zero;

    // 向量最小值和最大值
    static Vector2<T> Min(const Vector2<T>& vec1, const Vector2<T>& vec2);
    static Vector2<T> Max(const Vector2<T>& vec1, const Vector2<T>& vec2);

    void print() const;
};
using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;

const Vector2i Vector2i::zero = Vector2i(0, 0);
const Vector2f Vector2f::zero = Vector2f(0.0f, 0.0f);

template <typename T>
Vector2<T> Vector2<T>::operator+(const Vector2<T>& vec) const
{
    return Vector2<T>(x + vec.x, y + vec.y);
}

template <typename T>
Vector2<T> Vector2<T>::operator-(const Vector2<T>& vec) const
{
    return Vector2<T>(x - vec.x, y - vec.y);
}

template <typename T>
Vector2<T> Vector2<T>::operator-() const
{
    return Vector2<T>(-x, -y);
}

template <typename T>
Vector2<T> Vector2<T>::operator*(const float& scalar) const
{
    return Vector2<T>(x * scalar, y * scalar);
}

template <typename T>
Vector2<T> Vector2<T>::operator/(const float& scalar) const
{
    // 防止除0 - 改进版
    if (std::abs(scalar) < EPSILON)
        return Vector2<T>::zero; // 返回零向量
    
    float invScalar = 1.0f / scalar;
    return Vector2<T>(x * invScalar, y * invScalar);
}

template <typename T>
Vector2<T>& Vector2<T>::operator+=(const Vector2<T>& vec)
{
    x += vec.x;
    y += vec.y;
    return *this;
}

template <typename T>
Vector2<T>& Vector2<T>::operator-=(const Vector2<T>& vec)
{
    x -= vec.x;
    y -= vec.y;
    return *this;
}

template <typename T>
Vector2<T>& Vector2<T>::operator*=(const float& scalar)
{
    x *= scalar;
    y *= scalar;
    return *this;
}

template <typename T>
Vector2<T>& Vector2<T>::operator/=(const float& scalar)
{
    // 防止除0 - 改进版
    if (std::abs(scalar) < EPSILON)
        return *this; // 保持不变
    
    float invScalar = 1.0f / scalar;
    x *= invScalar;
    y *= invScalar;
    return *this;
}

template <typename T>
Vector2<T>& Vector2<T>::operator*=(const Vector2<T>& vec)
{
    x *= vec.x;
    y *= vec.y;
    return *this;
}

template <typename T>
float Vector2<T>::magnitudeSquared() const
{
    return x * x + y * y;
}

template <typename T>
float Vector2<T>::magnitude() const
{
    return sqrt(magnitudeSquared());
}

template <typename T>
Vector2<T>& Vector2<T>::normalize()
{
    float mag = magnitude();
    // 防止除0 - 改进版
    if (mag < EPSILON)
        return *this; // 长度太小，保持不变
        
    *this /= mag;
    return *this;
}

template <typename T>
float Vector2<T>::dot(const Vector2<T>& vec) const
{
    return x * vec.x + y * vec.y;
}

template <typename T>
float Vector2<T>::dot(const Vector2<T>& vec1, const Vector2<T>& vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y;
}

template <typename T>
Vector2<T> Vector2<T>::lerp(const Vector2<T>& vec1, const Vector2<T>& vec2, float weight)
{
    return vec1 + (vec2 - vec1) * weight;
}

template <typename T>
Vector2<T> Vector2<T>::standardization(const Vector2<T>& vec)
{
    float length = vec.magnitude();
    if (length < EPSILON) // 使用常量进行比较
    {
        return Vector2<T>(0, 0);
    }
    return vec / length;
}

template <typename T>
Vector2<T> Vector2<T>::reflect(const Vector2<T>& incident, const Vector2<T>& normal)
{
    // R = I - 2 * (I·N) * N
    return incident - normal * 2.0f * dot(incident, normal);
}

template <typename T>
Vector2<T> Vector2<T>::Min(const Vector2<T>& vec1, const Vector2<T>& vec2)
{
    return Vector2<T>((std::min)(vec1.x, vec2.x), (std::min)(vec1.y, vec2.y));
}

template <typename T>
Vector2<T> Vector2<T>::Max(const Vector2<T>& vec1, const Vector2<T>& vec2)
{
    return Vector2<T>((std::max)(vec1.x, vec2.x), (std::max)(vec1.y, vec2.y));
}

template <typename T>
void Vector2<T>::print() const
{
    std::cout << "Vector2(" << x << ", " << y << ")" << std::endl;
}



// 三维向量
template <typename T>
class Vector3
{
public:
    T x, y, z, w;

    Vector3() : x(0), y(0), z(0), w(1) {}
    Vector3(T x, T y, T z) : x(x), y(y), z(z), w(1) {}
    Vector3(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    Vector3(const Vector3<T>& vec) : x(vec.x), y(vec.y), z(vec.z), w(vec.w) {}
    
    ~Vector3() {}

public:
    Vector3<T> operator+(const Vector3<T>& vec) const;
    Vector3<T> operator-(const Vector3<T>& vec) const;
    Vector3<T> operator-() const;  // 单目减运算符
    Vector3<T> operator*(const float& scalar) const;
    Vector3<T> operator/(const float& scalar) const;

    // 复合赋值运算符
    Vector3<T>& operator+=(const Vector3<T>& vec);
    Vector3<T>& operator-=(const Vector3<T>& vec);
    Vector3<T>& operator*=(const float& scalar);
    Vector3<T>& operator/=(const float& scalar);
    Vector3<T>& operator*=(const Vector3<T>& vec);  // 向量分量相乘

    // 向量模长的平方
    float magnitudeSquared() const;

    // 向量模长
    float magnitude() const;

    // 向量归一化
    Vector3<T>& normalize();

    // 向量点乘
    float dot(const Vector3<T>& vec) const;
    static float dot(const Vector3<T>& vec1, const Vector3<T>& vec2);

    // 向量叉乘
    static Vector3<T> cross(const Vector3<T>& vec1, const Vector3<T>& vec2);
    
    // 向量插值
    static Vector3<T> lerp(const Vector3<T>& vec1, const Vector3<T>& vec2, float weight);

    // 向量反射
    static Vector3<T> reflect(const Vector3<T>& incident, const Vector3<T>& normal);

    // 零向量
    static const Vector3<T> zero;
    // 单位向量
    static const Vector3<T> one;

    // 向量最小值和最大值
    static Vector3<T> Min(const Vector3<T>& vec1, const Vector3<T>& vec2);
    static Vector3<T> Max(const Vector3<T>& vec1, const Vector3<T>& vec2);

    void print() const;
    void printWithW() const;
};
using Vector3f = Vector3<float>;
using Vector3i = Vector3<int>;

const Vector3i Vector3i::zero = Vector3i(0, 0, 0);
const Vector3f Vector3f::zero = Vector3f(0.0f, 0.0f, 0.0f);
const Vector3i Vector3i::one = Vector3i(1, 1, 1);
const Vector3f Vector3f::one = Vector3f(1.0f, 1.0f, 1.0f);

template <typename T>
Vector3<T> Vector3<T>::operator+(const Vector3<T>& vec) const
{
    return Vector3<T>(x + vec.x, y + vec.y, z + vec.z);
}

template <typename T>
Vector3<T> Vector3<T>::operator-(const Vector3<T>& vec) const
{
    return Vector3<T>(x - vec.x, y - vec.y, z - vec.z);
}

template <typename T>
Vector3<T> Vector3<T>::operator-() const
{
    return Vector3<T>(-x, -y, -z);
}

template <typename T>
Vector3<T> Vector3<T>::operator*(const float& scalar) const
{
    return Vector3<T>(x * scalar, y * scalar, z * scalar);
}

template <typename T>
Vector3<T> Vector3<T>::operator/(const float& scalar) const
{
    // 防止除0 - 改进版
    if (std::abs(scalar) < EPSILON)
        return Vector3<T>::zero; // 返回零向量
    
    float invScalar = 1.0f / scalar;
    return Vector3<T>(x * invScalar, y * invScalar, z * invScalar);
}

template <typename T>
Vector3<T>& Vector3<T>::operator+=(const Vector3<T>& vec)
{
    x += vec.x;
    y += vec.y;
    z += vec.z;
    return *this;
}

template <typename T>
Vector3<T>& Vector3<T>::operator-=(const Vector3<T>& vec)
{
    x -= vec.x;
    y -= vec.y;
    z -= vec.z;
    return *this;
}

template <typename T>
Vector3<T>& Vector3<T>::operator*=(const float& scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

template <typename T>
Vector3<T>& Vector3<T>::operator/=(const float& scalar)
{
    // 防止除0 - 改进版
    if (std::abs(scalar) < EPSILON)
        return *this; // 保持不变
    
    float invScalar = 1.0f / scalar;
    x *= invScalar;
    y *= invScalar;
    z *= invScalar;
    return *this;
}

template <typename T>
Vector3<T>& Vector3<T>::operator*=(const Vector3<T>& vec)
{
    x *= vec.x;
    y *= vec.y;
    z *= vec.z;
    return *this;
}

template <typename T>
float Vector3<T>::magnitudeSquared() const
{
    return x * x + y * y + z * z;
}

template <typename T>
float Vector3<T>::magnitude() const
{
    return sqrt(magnitudeSquared());
}   

template <typename T>
Vector3<T>& Vector3<T>::normalize()
{
    float mag = magnitude();
    // 防止除0 - 改进版
    if (mag < EPSILON)
        return *this; // 长度太小，保持不变
        
    *this = *this / mag;
    return *this;
}

template <typename T>
float Vector3<T>::dot(const Vector3<T>& vec) const
{
    return x * vec.x + y * vec.y + z * vec.z;
}

template <typename T>
float Vector3<T>::dot(const Vector3<T>& vec1, const Vector3<T>& vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
}

template <typename T>
Vector3<T> Vector3<T>::cross(const Vector3<T>& vec1, const Vector3<T>& vec2)
{
    return Vector3<T>(vec1.y * vec2.z - vec1.z * vec2.y, vec1.z * vec2.x - vec1.x * vec2.z, vec1.x * vec2.y - vec1.y * vec2.x);
}

template <typename T>
Vector3<T> Vector3<T>::lerp(const Vector3<T>& vec1, const Vector3<T>& vec2, float weight)
{
    return vec1 + (vec2 - vec1) * weight;
}

template <typename T>
Vector3<T> Vector3<T>::reflect(const Vector3<T>& incident, const Vector3<T>& normal)
{
    // R = I - 2 * (I·N) * N
    return incident - normal * 2.0f * dot(incident, normal);
}

template <typename T>
Vector3<T> Vector3<T>::Min(const Vector3<T>& vec1, const Vector3<T>& vec2)
{
    return Vector3<T>(
        (std::min)(vec1.x, vec2.x),
        (std::min)(vec1.y, vec2.y),
        (std::min)(vec1.z, vec2.z)
    );
}

template <typename T>
Vector3<T> Vector3<T>::Max(const Vector3<T>& vec1, const Vector3<T>& vec2)
{
    return Vector3<T>(
        (std::max)(vec1.x, vec2.x),
        (std::max)(vec1.y, vec2.y),
        (std::max)(vec1.z, vec2.z)
    );
}

template <typename T>
void Vector3<T>::print() const
{
    std::cout << "Vector3(" << x << ", " << y << ", " << z << ")" << std::endl;
}

template <typename T>
void Vector3<T>::printWithW() const
{
    std::cout << "Vector3(" << x << ", " << y << ", " << z << ", " << w << ")" << std::endl;
}



// 四维向量
template <typename T>
class Vector4
{
public:
    T x, y, z, w;

    Vector4() : x(0), y(0), z(0), w(1) {}
    Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    Vector4(const Vector3<T>& vec, T w) : x(vec.x), y(vec.y), z(vec.z), w(w) {}
    Vector4(const Vector4<T>& vec) : x(vec.x), y(vec.y), z(vec.z), w(vec.w) {}
    
    ~Vector4() {}

public:
    Vector4<T> operator+(const Vector4<T>& vec) const;
    Vector4<T> operator-(const Vector4<T>& vec) const;
    Vector4<T> operator-() const;  // 单目减运算符
    Vector4<T> operator*(const float& scalar) const;
    Vector4<T> operator/(const float& scalar) const;

    // 复合赋值运算符
    Vector4<T>& operator+=(const Vector4<T>& vec);
    Vector4<T>& operator-=(const Vector4<T>& vec);
    Vector4<T>& operator*=(const float& scalar);
    Vector4<T>& operator/=(const float& scalar);
    Vector4<T>& operator*=(const Vector4<T>& vec);  // 向量分量相乘

    // 向量模长的平方
    float magnitudeSquared() const;

    // 向量模长
    float magnitude() const;    

    // 向量归一化
    Vector4<T>& normalize();

    // 向量点乘
    float dot(const Vector4<T>& vec) const;
    static float dot(const Vector4<T>& vec1, const Vector4<T>& vec2);

    // 向量插值
    static Vector4<T> lerp(const Vector4<T>& vec1, const Vector4<T>& vec2, float weight);

    // 向量反射
    static Vector4<T> reflect(const Vector4<T>& incident, const Vector4<T>& normal);

    // 零向量
    static const Vector4<T> zero;

    // 向量最小值和最大值
    static Vector4<T> Min(const Vector4<T>& vec1, const Vector4<T>& vec2);
    static Vector4<T> Max(const Vector4<T>& vec1, const Vector4<T>& vec2);

    void print() const;

};
using Vector4f = Vector4<float>;    
using Vector4i = Vector4<int>;

const Vector4i Vector4i::zero = Vector4i(0, 0, 0, 0);
const Vector4f Vector4f::zero = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);

template <typename T>
Vector4<T> Vector4<T>::operator+(const Vector4<T>& vec) const
{
    return Vector4<T>(x + vec.x, y + vec.y, z + vec.z, w + vec.w);
}

template <typename T>
Vector4<T> Vector4<T>::operator-(const Vector4<T>& vec) const
{
    return Vector4<T>(x - vec.x, y - vec.y, z - vec.z, w - vec.w);
}

template <typename T>
Vector4<T> Vector4<T>::operator-() const
{
    return Vector4<T>(-x, -y, -z, -w);
}

template <typename T>
Vector4<T> Vector4<T>::operator*(const float& scalar) const
{
    return Vector4<T>(x * scalar, y * scalar, z * scalar, w * scalar);
}

template <typename T>
Vector4<T> Vector4<T>::operator/(const float& scalar) const
{
    // 防止除0 - 改进版
    if (std::abs(scalar) < EPSILON)
        return Vector4<T>::zero; // 返回零向量
    
    float invScalar = 1.0f / scalar;
    return Vector4<T>(x * invScalar, y * invScalar, z * invScalar, w * invScalar);
}

template <typename T>
Vector4<T>& Vector4<T>::operator+=(const Vector4<T>& vec)
{
    x += vec.x;
    y += vec.y;
    z += vec.z;
    w += vec.w;
    return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator-=(const Vector4<T>& vec)
{
    x -= vec.x;
    y -= vec.y;
    z -= vec.z;
    w -= vec.w;
    return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator*=(const float& scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator/=(const float& scalar)
{
    // 防止除0 - 改进版
    if (std::abs(scalar) < EPSILON)
        return *this; // 保持不变
    
    float invScalar = 1.0f / scalar;
    x *= invScalar;
    y *= invScalar;
    z *= invScalar;
    w *= invScalar;
    return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator*=(const Vector4<T>& vec)
{
    x *= vec.x;
    y *= vec.y;
    z *= vec.z;
    w *= vec.w;
    return *this;
}

template <typename T>
float Vector4<T>::magnitudeSquared() const
{
    return x * x + y * y + z * z + w * w;
}

template <typename T>
float Vector4<T>::magnitude() const
{
    return sqrt(magnitudeSquared());
}

template <typename T>
Vector4<T>& Vector4<T>::normalize()
{
    float mag = magnitude();
    // 防止除0 - 改进版
    if (mag < EPSILON)
        return *this; // 长度太小，保持不变
        
    *this /= mag;
    return *this;
}

template <typename T>
float Vector4<T>::dot(const Vector4<T>& vec) const
{
    return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
}

template <typename T>
float Vector4<T>::dot(const Vector4<T>& vec1, const Vector4<T>& vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z + vec1.w * vec2.w;
}

template <typename T>
Vector4<T> Vector4<T>::lerp(const Vector4<T>& vec1, const Vector4<T>& vec2, float weight)
{
    return vec1 + (vec2 - vec1) * weight;
}

template <typename T>
Vector4<T> Vector4<T>::reflect(const Vector4<T>& incident, const Vector4<T>& normal)
{
    // R = I - 2 * (I·N) * N
    return incident - normal * 2.0f * dot(incident, normal);
}

template <typename T>
Vector4<T> Vector4<T>::Min(const Vector4<T>& vec1, const Vector4<T>& vec2)
{
    return Vector4<T>(
        (std::min)(vec1.x, vec2.x),
        (std::min)(vec1.y, vec2.y),
        (std::min)(vec1.z, vec2.z),
        (std::min)(vec1.w, vec2.w)
    );
}

template <typename T>
Vector4<T> Vector4<T>::Max(const Vector4<T>& vec1, const Vector4<T>& vec2)
{
    return Vector4<T>(
        (std::max)(vec1.x, vec2.x),
        (std::max)(vec1.y, vec2.y),
        (std::max)(vec1.z, vec2.z),
        (std::max)(vec1.w, vec2.w)
    );
}

template <typename T>
void Vector4<T>::print() const
{
    std::cout << "Vector4(" << x << ", " << y << ", " << z << ", " << w << ")" << std::endl;
}

