#pragma once

#include "Matrix.h"
#include "MyMath.h"
#include "Vector.h"
#include "Color.h"
#include <vector>
#include <string>

// 顶点
class Vertex
{
public:
    Vector4f pos; // 位置
    Vector4f color; // 颜色
    Vector3f normal; // 法线
    Vector2f texcoord; // 纹理坐标

public:
    Vertex() : pos(), color(), normal(), texcoord() {}
    // 位置构造
    Vertex(const Vector4f& pos) : pos(pos), color(), normal(), texcoord() {}
    
    // 位置和颜色构造
    Vertex(const Vector4f& pos, const Vector4f& color) : pos(pos), color(color), normal(), texcoord() {}
    
    // 位置、颜色和法线构造
    Vertex(const Vector4f& pos, const Vector4f& color, const Vector3f& normal) 
        : pos(pos), color(color), normal(normal), texcoord() {}
    
    // 位置、颜色、法线和纹理坐标构造
    Vertex(const Vector4f& pos, const Vector4f& color, const Vector3f& normal, const Vector2f& texcoord)
        : pos(pos), color(color), normal(normal), texcoord(texcoord) {}
        
    // Todo: 颜色可以用已经实现的颜色类来构造Vertex

    // 拷贝构造函数
    Vertex(const Vertex& vertex) 
        : pos(vertex.pos), color(vertex.color), normal(vertex.normal), texcoord(vertex.texcoord) {}


    // 顶点插值函数 - 线性插值两个顶点
    static Vertex lerp(const Vertex& v1, const Vertex& v2, float weight)
    {
        Vertex result;
        // 插值位置
        result.pos = Vector4f::lerp(v1.pos, v2.pos, weight);
        // 插值颜色
        result.color = Vector4f::lerp(v1.color, v2.color, weight);
        // 插值法线（注意：插值后需要重新归一化）
        result.normal = Vector3f::lerp(v1.normal, v2.normal, weight).normalize();
        // 插值纹理坐标
        result.texcoord = Vector2f::lerp(v1.texcoord, v2.texcoord, weight);
        return result;
    }
    
};

// 网格
class Mesh
{
public:
    std::vector<Vertex> vertices; // 顶点数组
    std::vector<Vector3i> indices; // 顶点索引数组
    
public:
    Mesh() : vertices(), indices() {}
    
    Mesh(const std::vector<Vertex>& verts, const std::vector<Vector3i>& inds)
        : vertices(verts), indices(inds) {}
    
    // 拷贝构造函数
    Mesh(const Mesh& other)
        : vertices(other.vertices), indices(other.indices) {}
    
    // 添加顶点
    void AddVertex(const Vertex& vertex) {
        vertices.push_back(vertex);
    }
    
    // 添加索引
    void AddIndex(const Vector3i& index) {
        indices.push_back(index);
    }
    
    // 添加三角形（通过三个索引）
    void AddTriangle(unsigned int i1, unsigned int i2, unsigned int i3) {
        indices.push_back(Vector3i(i1, i2, i3));
    }

    void AddTriangle(Vector3i indice)
    {
        indices.push_back(indice);
    }
    
    // 清空网格数据
    void Clear() {
        vertices.clear();
        indices.clear();
    }
    
    // 获取顶点数量
    size_t GetVertexCount() const {
        return vertices.size();
    }
    
    // 获取索引数量
    size_t GetIndexCount() const {
        return indices.size() * 3;
    }
    
    // 获取三角形数量
    size_t GetTriangleCount() const {
        return indices.size();
    }

    // 计算网格的边界框
    void CalculateBounds(Vector3f& min, Vector3f& max) const {
        if (vertices.empty()) {
            min = max = Vector3f::zero;
            return;
        }

        min = max = Vector3f(vertices[0].pos.x, vertices[0].pos.y, vertices[0].pos.z);
        for (const auto& vertex : vertices) {
            min = Vector3f::Min(min, Vector3f(vertex.pos.x, vertex.pos.y, vertex.pos.z));
            max = Vector3f::Max(max, Vector3f(vertex.pos.x, vertex.pos.y, vertex.pos.z));
        }
    }

    // 计算网格的中心点
    Vector3f CalculateCenter() const {
        if (vertices.empty()) {
            return Vector3f::zero;
        }

        Vector3f sum = Vector3f::zero;
        for (const auto& vertex : vertices) {
            sum += Vector3f(vertex.pos.x, vertex.pos.y, vertex.pos.z);
        }
        return sum / static_cast<float>(vertices.size());
    }

    // 计算网格的包围球
    void CalculateBoundingSphere(Vector3f& center, float& radius) const {
        center = CalculateCenter();
        radius = 0.0f;

        for (const auto& vertex : vertices) {
            Vector3f pos(vertex.pos.x, vertex.pos.y, vertex.pos.z);
            float distance = (pos - center).magnitude();
            if (distance > radius) {
                radius = distance;
            }
        }
    }

    // 计算法线（如果顶点没有法线）
    void CalculateNormals() {
        // 首先重置所有法线
        for (auto& vertex : vertices) {
            vertex.normal = Vector3f::zero;
        }

        // 计算每个三角形的法线并累加到顶点
        for (const auto& index : indices) {
            const Vertex& v0 = vertices[index.x];
            const Vertex& v1 = vertices[index.y];
            const Vertex& v2 = vertices[index.z];

            Vector3f p0(v0.pos.x, v0.pos.y, v0.pos.z);
            Vector3f p1(v1.pos.x, v1.pos.y, v1.pos.z);
            Vector3f p2(v2.pos.x, v2.pos.y, v2.pos.z);

            Vector3f normal = Vector3f::cross(p1 - p0, p2 - p0).normalize();

            vertices[index.x].normal += normal;
            vertices[index.y].normal += normal;
            vertices[index.z].normal += normal;
        }

        // 归一化所有法线
        for (auto& vertex : vertices) {
            vertex.normal = vertex.normal.normalize();
        }
    }
};

// 材质类
class Material {
public:
    Color ambient;      // 环境光颜色
    Color diffuse;      // 漫反射颜色
    Color specular;     // 镜面反射颜色
    float shininess;    // 高光指数
    float opacity;      // 不透明度

    Material() 
        : ambient(Color::white), 
          diffuse(Color::white), 
          specular(Color::white), 
          shininess(32.0f),
          opacity(1.0f) {}

    Material(const Color& ambient, const Color& diffuse, const Color& specular, float shininess, float opacity = 1.0f)
        : ambient(ambient), 
          diffuse(diffuse), 
          specular(specular), 
          shininess(shininess),
          opacity(opacity) {}
};

// 矩阵变换类
class Transformer {
public:
    Vector3f position;      // 位置
    Vector3f rotation;      // 旋转（欧拉角，单位：度）
    Vector3f scale;         // 缩放

    Transformer() 
        : position(Vector3f::zero), 
          rotation(Vector3f::zero), 
          scale(Vector3f::one) {}

    // 获取模型矩阵
    Matrix GetModelMatrix() const {
        // 按照缩放->旋转->平移的顺序构建矩阵
        // 注意：矩阵乘法顺序很重要，后乘的变换先应用
        Matrix result = Matrix::identity();
        Matrix scaleMatrix = Matrix::scale(scale);
        Matrix rotationMatrixX = Matrix::rotate(rotation.x, 'x');
        Matrix rotationMatrixY = Matrix::rotate(rotation.y, 'y');
        Matrix rotationMatrixZ = Matrix::rotate(rotation.z, 'z');
        Matrix translationMatrix = Matrix::translate(position);
        
        // 正确的矩阵乘法顺序：平移 * 旋转Z * 旋转Y * 旋转X * 缩放
        result = translationMatrix * rotationMatrixZ * rotationMatrixY * rotationMatrixX * scaleMatrix;
        return result;
    }

    // 设置位置
    void SetPosition(const Vector3f& pos) {
        position = pos;
    }

    // 设置旋转
    void SetRotation(const Vector3f& rot) {
        rotation = rot;
    }

    // 设置缩放
    void SetScale(const Vector3f& scl) {
        scale = scl;
    }

    // 平移
    void Translate(const Vector3f& translation) {
        position += translation;
    }

    // 旋转
    void Rotate(const Vector3f& rotation) {
        this->rotation += rotation;
    }

    // 缩放
    void Scale(const Vector3f& scale) {
        this->scale *= scale;
    }
};

// 对象类
class Object {
public:
    Mesh mesh;              // 网格数据
    Material material;      // 材质
    Transformer transform;  // 变换

    Object() : mesh(), material(), transform() {}
    Object(const Mesh& mesh, const Material& material, const Transformer& transform)
        : mesh(mesh), material(material), transform(transform) {}

    // 获取模型矩阵
    Matrix GetModelMatrix() const {
        return transform.GetModelMatrix();
    }

    // 设置位置
    void SetPosition(const Vector3f& position) {
        transform.SetPosition(position);
    }

    // 设置旋转
    void SetRotation(const Vector3f& rotation) {
        transform.SetRotation(rotation);
    }

    // 设置缩放
    void SetScale(const Vector3f& scale) {
        transform.SetScale(scale);
    }

    // 平移
    void Translate(const Vector3f& translation) {
        transform.Translate(translation);
    }

    // 旋转
    void Rotate(const Vector3f& rotation) {
        transform.Rotate(rotation);
    }

    // 缩放
    void Scale(const Vector3f& scale) {
        transform.Scale(scale);
    }

    // 计算对象的边界框
    void CalculateBounds(Vector3f& min, Vector3f& max) const {
        mesh.CalculateBounds(min, max);
        // 应用变换
        Matrix modelMatrix = GetModelMatrix();
        Vector4f minVec(min, 1.0f);
        Vector4f maxVec(max, 1.0f);
        minVec = modelMatrix * minVec;
        maxVec = modelMatrix * maxVec;
        min = Vector3f(minVec.x, minVec.y, minVec.z);
        max = Vector3f(maxVec.x, maxVec.y, maxVec.z);
    }

    // 计算对象的中心点
    Vector3f CalculateCenter() const {
        Vector3f center = mesh.CalculateCenter();
        Matrix modelMatrix = GetModelMatrix();
        Vector4f centerVec(center.x, center.y, center.z, 1.0f);
        centerVec = modelMatrix * centerVec;
        return Vector3f(centerVec.x, centerVec.y, centerVec.z);
    }

    // 计算对象的包围球
    void CalculateBoundingSphere(Vector3f& center, float& radius) const {
        mesh.CalculateBoundingSphere(center, radius);
        // 应用变换
        Matrix modelMatrix = GetModelMatrix();
        Vector4f centerVec(center.x, center.y, center.z, 1.0f);
        centerVec = modelMatrix * centerVec;
        center = Vector3f(centerVec.x, centerVec.y, centerVec.z);
        // 考虑缩放对半径的影响
        float maxScale = (std::max)((std::max)(transform.scale.x, transform.scale.y), transform.scale.z);
        radius *= maxScale;
    }
};