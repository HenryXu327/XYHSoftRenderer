#pragma once
#include "Vector.h"


class Matrix
{
public:
    float m[4][4];

public:
    Matrix();
    Matrix(const Matrix& matrix);
    Matrix(const float matrix[4][4]);

    ~Matrix();

    Matrix operator+(const Matrix& matrix) const;
    Matrix operator-(const Matrix& matrix) const;
    Matrix operator*(const Matrix& matrix) const; // 矩阵乘矩阵
    Vector4f operator*(const Vector4f& vector) const; // 矩阵乘向量
    Matrix operator*(const float& scalar) const; // 矩阵乘标量
    Matrix operator/(const float& scalar) const; // 矩阵除标量

    Matrix& operator=(const Matrix& matrix); // 矩阵赋值

    static Matrix identity(); // 单位矩阵

    Matrix transpose() const; // 矩阵转置
    Matrix inverse() const; // 矩阵逆

    static Matrix translate(const Vector3f& translation); // 平移矩阵
    static Matrix rotate(float angle, char axis); // xyz轴旋转矩阵
    static Matrix rotate(float angle, const Vector3f& axis); // 任意轴旋转矩阵
    static Matrix scale(const Vector3f& scale); // 缩放矩阵

    // 创建透视投影矩阵
    static Matrix perspective(float fov, float aspect, float nearZ, float farZ);

    // 创建相机视图矩阵
    static Matrix lookAt(const Vector3f& eye, const Vector3f& center, const Vector3f& up);

    void print() const; // 打印矩阵
};
