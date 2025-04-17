#include "../include/Matrix.h"
#include "../include/MyMath.h"

Matrix::Matrix()
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            m[i][j] = 0.0f;
}

Matrix::Matrix(const Matrix& matrix)
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            m[i][j] = matrix.m[i][j];
}

Matrix::Matrix(const float matrix[4][4])
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            m[i][j] = matrix[i][j];
}

Matrix::~Matrix()
{
}

Matrix Matrix::operator+(const Matrix& matrix) const
{
    Matrix result;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            result.m[i][j] = m[i][j] + matrix.m[i][j];
    return result;
}

Matrix Matrix::operator-(const Matrix& matrix) const
{
    Matrix result;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            result.m[i][j] = m[i][j] - matrix.m[i][j];
    return result;
}
// 最普通的矩阵乘法
// Matrix Matrix::operator*(const Matrix& matrix) const
// {
//     Matrix result;
//     for (int i = 0; i < 4; i++) {
//         for (int j = 0; j < 4; j++) {
//             result.m[i][j] = 0.0f;
//             for (int k = 0; k < 4; k++) {
//                 result.m[i][j] += m[i][k] * matrix.m[k][j];
//             }
//         }
//     }
//     return result;
// }

// 优化后的矩阵乘法
Matrix Matrix::operator*(const Matrix& matrix) const
{
    Matrix result;
    
    // 使用局部变量缓存行和列，避免重复内存访问
    // 针对4x4矩阵完全展开循环，消除循环开销
    
    // 第一行
    const float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3];
    const float b00 = matrix.m[0][0], b10 = matrix.m[1][0];
    const float b20 = matrix.m[2][0], b30 = matrix.m[3][0];
    result.m[0][0] = a00 * b00 + a01 * b10 + a02 * b20 + a03 * b30;
    
    const float b01 = matrix.m[0][1], b11 = matrix.m[1][1];
    const float b21 = matrix.m[2][1], b31 = matrix.m[3][1];
    result.m[0][1] = a00 * b01 + a01 * b11 + a02 * b21 + a03 * b31;
    
    const float b02 = matrix.m[0][2], b12 = matrix.m[1][2];
    const float b22 = matrix.m[2][2], b32 = matrix.m[3][2];
    result.m[0][2] = a00 * b02 + a01 * b12 + a02 * b22 + a03 * b32;
    
    const float b03 = matrix.m[0][3], b13 = matrix.m[1][3];
    const float b23 = matrix.m[2][3], b33 = matrix.m[3][3];
    result.m[0][3] = a00 * b03 + a01 * b13 + a02 * b23 + a03 * b33;
    
    // 第二行
    const float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3];
    result.m[1][0] = a10 * b00 + a11 * b10 + a12 * b20 + a13 * b30;
    result.m[1][1] = a10 * b01 + a11 * b11 + a12 * b21 + a13 * b31;
    result.m[1][2] = a10 * b02 + a11 * b12 + a12 * b22 + a13 * b32;
    result.m[1][3] = a10 * b03 + a11 * b13 + a12 * b23 + a13 * b33;
    
    // 第三行
    const float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3];
    result.m[2][0] = a20 * b00 + a21 * b10 + a22 * b20 + a23 * b30;
    result.m[2][1] = a20 * b01 + a21 * b11 + a22 * b21 + a23 * b31;
    result.m[2][2] = a20 * b02 + a21 * b12 + a22 * b22 + a23 * b32;
    result.m[2][3] = a20 * b03 + a21 * b13 + a22 * b23 + a23 * b33;
    
    // 第四行
    const float a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3];
    result.m[3][0] = a30 * b00 + a31 * b10 + a32 * b20 + a33 * b30;
    result.m[3][1] = a30 * b01 + a31 * b11 + a32 * b21 + a33 * b31;
    result.m[3][2] = a30 * b02 + a31 * b12 + a32 * b22 + a33 * b32;
    result.m[3][3] = a30 * b03 + a31 * b13 + a32 * b23 + a33 * b33;
    
    return result;
}

Vector4f Matrix::operator*(const Vector4f& vector) const
{
    // 使用局部变量缓存，减少内存访问
    const float x = vector.x, y = vector.y, z = vector.z, w = vector.w;
    
    // 使用行主序计算，提高缓存命中率
    const float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
    const float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
    const float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
    const float m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];
    
    return Vector4f(
        m00 * x + m01 * y + m02 * z + m03 * w,
        m10 * x + m11 * y + m12 * z + m13 * w,
        m20 * x + m21 * y + m22 * z + m23 * w,
        m30 * x + m31 * y + m32 * z + m33 * w
    );
}

Matrix Matrix::operator*(const float& scalar) const
{
    Matrix result;
    
    // 展开循环，减少循环开销，直接计算所有元素
    result.m[0][0] = m[0][0] * scalar;
    result.m[0][1] = m[0][1] * scalar;
    result.m[0][2] = m[0][2] * scalar;
    result.m[0][3] = m[0][3] * scalar;
    
    result.m[1][0] = m[1][0] * scalar;
    result.m[1][1] = m[1][1] * scalar;
    result.m[1][2] = m[1][2] * scalar;
    result.m[1][3] = m[1][3] * scalar;
    
    result.m[2][0] = m[2][0] * scalar;
    result.m[2][1] = m[2][1] * scalar;
    result.m[2][2] = m[2][2] * scalar;
    result.m[2][3] = m[2][3] * scalar;
    
    result.m[3][0] = m[3][0] * scalar;
    result.m[3][1] = m[3][1] * scalar;
    result.m[3][2] = m[3][2] * scalar;
    result.m[3][3] = m[3][3] * scalar;
    
    return result;
}

Matrix Matrix::operator/(const float& scalar) const
{
    assert(scalar != 0.0f);
    float invScalar = 1.0f / scalar;
    return (*this) * invScalar;
}

Matrix& Matrix::operator=(const Matrix& matrix)
{
    if (this != &matrix) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                m[i][j] = matrix.m[i][j];
    }
    return *this;
}

Matrix Matrix::identity()
{
    Matrix result;
    for (int i = 0; i < 4; i++)
        result.m[i][i] = 1.0f;
    return result;
}

Matrix Matrix::transpose() const
{
    Matrix result;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            result.m[i][j] = m[j][i];
    return result;
}

// 矩阵求逆（注意：之前用的高斯约旦求矩阵逆的方法没有考虑矩阵不可逆的情况，导致在特定旋转角度（如90度、270度）下会造成光的闪烁）
Matrix Matrix::inverse() const
{
    Matrix result;
    
    // 针对4x4矩阵的快速求逆算法
    // 计算行列式的余子式
    float s0 = m[0][0] * m[1][1] - m[1][0] * m[0][1];
    float s1 = m[0][0] * m[1][2] - m[1][0] * m[0][2];
    float s2 = m[0][0] * m[1][3] - m[1][0] * m[0][3];
    float s3 = m[0][1] * m[1][2] - m[1][1] * m[0][2];
    float s4 = m[0][1] * m[1][3] - m[1][1] * m[0][3];
    float s5 = m[0][2] * m[1][3] - m[1][2] * m[0][3];

    float c5 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
    float c4 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
    float c3 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
    float c2 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
    float c1 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
    float c0 = m[2][0] * m[3][1] - m[3][0] * m[2][1];

    // 计算行列式的值
    float det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
    
    // 行列式为0表示矩阵不可逆
    if (std::abs(det) < EPSILON) {
        // 返回单位矩阵
        return identity();
    }

    // 计算行列式的倒数
    float invDet = 1.0f / det;

    // 计算伴随矩阵的转置
    result.m[0][0] = ( m[1][1] * c5 - m[1][2] * c4 + m[1][3] * c3) * invDet;
    result.m[0][1] = (-m[0][1] * c5 + m[0][2] * c4 - m[0][3] * c3) * invDet;
    result.m[0][2] = ( m[3][1] * s5 - m[3][2] * s4 + m[3][3] * s3) * invDet;
    result.m[0][3] = (-m[2][1] * s5 + m[2][2] * s4 - m[2][3] * s3) * invDet;

    result.m[1][0] = (-m[1][0] * c5 + m[1][2] * c2 - m[1][3] * c1) * invDet;
    result.m[1][1] = ( m[0][0] * c5 - m[0][2] * c2 + m[0][3] * c1) * invDet;
    result.m[1][2] = (-m[3][0] * s5 + m[3][2] * s2 - m[3][3] * s1) * invDet;
    result.m[1][3] = ( m[2][0] * s5 - m[2][2] * s2 + m[2][3] * s1) * invDet;

    result.m[2][0] = ( m[1][0] * c4 - m[1][1] * c2 + m[1][3] * c0) * invDet;
    result.m[2][1] = (-m[0][0] * c4 + m[0][1] * c2 - m[0][3] * c0) * invDet;
    result.m[2][2] = ( m[3][0] * s4 - m[3][1] * s2 + m[3][3] * s0) * invDet;
    result.m[2][3] = (-m[2][0] * s4 + m[2][1] * s2 - m[2][3] * s0) * invDet;

    result.m[3][0] = (-m[1][0] * c3 + m[1][1] * c1 - m[1][2] * c0) * invDet;
    result.m[3][1] = ( m[0][0] * c3 - m[0][1] * c1 + m[0][2] * c0) * invDet;
    result.m[3][2] = (-m[3][0] * s3 + m[3][1] * s1 - m[3][2] * s0) * invDet;
    result.m[3][3] = ( m[2][0] * s3 - m[2][1] * s1 + m[2][2] * s0) * invDet;

    return result;
}

// 平移矩阵
Matrix Matrix::translate(const Vector3f& translation)
{
    Matrix result = identity();
    result.m[0][3] = translation.x;
    result.m[1][3] = translation.y;
    result.m[2][3] = translation.z;
    return result;
}

// 返回一个旋转矩阵（绕xyz轴旋转）
Matrix Matrix::rotate(float angle, char axis)
{
    // 角度转弧度
    float radians = toRadians(angle);
    Matrix result = identity();
    
    // 使用右手坐标系规则
    switch (axis) {
        case 'x':
            // 绕X轴旋转
            result.m[1][1] = cosf(radians);
            result.m[1][2] = -sinf(radians);
            result.m[2][1] = sinf(radians);
            result.m[2][2] = cosf(radians);
            break;
        case 'y':
            // 绕Y轴旋转
            result.m[0][0] = cosf(radians);
            result.m[0][2] = sinf(radians);
            result.m[2][0] = -sinf(radians);
            result.m[2][2] = cosf(radians);
            break;
        case 'z':
            // 绕Z轴旋转
            result.m[0][0] = cosf(radians);
            result.m[0][1] = -sinf(radians);
            result.m[1][0] = sinf(radians);
            result.m[1][1] = cosf(radians);
            break;
        default:
            assert(false);
    }
    
    return result;
}

// 返回一个旋转矩阵（绕任意轴旋转）（axis参数必须是单位向量）
Matrix Matrix::rotate(float angle, const Vector3f& axis)
{
    // 角度转弧度
    float radians = toRadians(angle);
    Matrix result = identity();
    // 确保轴向量是单位向量
    Vector3f unitAxis = axis;
    unitAxis.normalize();
    // 罗德里格斯公式，计算旋转矩阵
    result.m[0][0] = cosf(radians) + unitAxis.x * unitAxis.x * (1 - cosf(radians));
    result.m[0][1] = unitAxis.x * unitAxis.y * (1 - cosf(radians)) - unitAxis.z * sinf(radians);
    result.m[0][2] = unitAxis.x * unitAxis.z * (1 - cosf(radians)) + unitAxis.y * sinf(radians);
    result.m[1][0] = unitAxis.y * unitAxis.x * (1 - cosf(radians)) + unitAxis.z * sinf(radians);
    result.m[1][1] = cosf(radians) + unitAxis.y * unitAxis.y * (1 - cosf(radians));
    result.m[1][2] = unitAxis.y * unitAxis.z * (1 - cosf(radians)) - unitAxis.x * sinf(radians);
    result.m[2][0] = unitAxis.z * unitAxis.x * (1 - cosf(radians)) - unitAxis.y * sinf(radians);
    result.m[2][1] = unitAxis.z * unitAxis.y * (1 - cosf(radians)) + unitAxis.x * sinf(radians);
    result.m[2][2] = cosf(radians) + unitAxis.z * unitAxis.z * (1 - cosf(radians));
    return result;
}

// 返回一个缩放矩阵
Matrix Matrix::scale(const Vector3f& scale)
{
    Matrix result = identity();
    result.m[0][0] = scale.x;
    result.m[1][1] = scale.y;
    result.m[2][2] = scale.z;
    return result;
}

void Matrix::print() const
{
    std::cout << "Matrix:" << std::endl;
	for (int i = 0; i < 4; i++) {
		std::cout << "  [";
		for (int j = 0; j < 4; j++) {
			std::cout << m[i][j] << " ";
		}
		std::cout << "]" << std::endl;
	}
    std::cout << std::endl;
}

// 创建透视投影矩阵
Matrix Matrix::perspective(float fov, float aspect, float nearZ, float farZ)
{
    Matrix result;
    float tanHalfFov = tanf(fov / 2.0f);

    result.m[0][0] = 1.0f / (aspect * tanHalfFov);
    result.m[1][1] = 1.0f / tanHalfFov;
    result.m[2][2] = (nearZ + farZ) / (nearZ - farZ);
    result.m[2][3] = (2.0f * nearZ * farZ) / (nearZ - farZ);
    result.m[3][2] = -1.0f;
    result.m[3][3] = 0.0f;

    return result;
}

// 创建相机视图矩阵
Matrix Matrix::lookAt(const Vector3f& camPos, const Vector3f& center, const Vector3f& up)
{
    Matrix result;
    
    // 计算相机坐标系的三个轴
    Vector3f zAxis = (camPos - center).normalize();
    Vector3f xAxis = Vector3f::cross(up, zAxis).normalize();
    Vector3f yAxis = Vector3f::cross(zAxis, xAxis).normalize();
    
    // 构建视图矩阵
    result.m[0][0] = xAxis.x;
    result.m[0][1] = xAxis.y;
    result.m[0][2] = xAxis.z;
    result.m[0][3] = -Vector3f::dot(xAxis, camPos);
    
    result.m[1][0] = yAxis.x;
    result.m[1][1] = yAxis.y;
    result.m[1][2] = yAxis.z;
    result.m[1][3] = -Vector3f::dot(yAxis, camPos);
    
    result.m[2][0] = zAxis.x;
    result.m[2][1] = zAxis.y;
    result.m[2][2] = zAxis.z;
    result.m[2][3] = -Vector3f::dot(zAxis, camPos);
    
    result.m[3][0] = 0.0f;
    result.m[3][1] = 0.0f;
    result.m[3][2] = 0.0f;
    result.m[3][3] = 1.0f;
    
    return result;
}
