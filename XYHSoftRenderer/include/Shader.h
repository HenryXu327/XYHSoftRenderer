#pragma once

#include "Vector.h"
#include "Matrix.h"
#include "Color.h"
#include "Object.h"
#include "MyMath.h"
#include "Texture.h"

// 顶点着色器输入
struct VertexShaderInput
{
    Vector4f position;    // 顶点位置
    Vector4f color;       // 顶点颜色
    Vector3f normal;      // 顶点法线
    Vector2f texcoord;    // 纹理坐标
    Matrix modelMatrix;   // 模型矩阵(M)（局部坐标系到世界坐标系）
    Matrix viewMatrix;    // 视图矩阵(V)（世界坐标系到相机坐标系）
    Matrix projMatrix;    // 投影矩阵(P)（相机坐标系到裁剪坐标系）
};

// 顶点着色器输出/片元着色器输入
struct VertexOutput
{
    Vector4f position;    // 裁剪空间位置
    Vector4f color;       // 插值颜色
    Vector3f normal;      // 世界空间法线
    Vector2f texcoord;    // 纹理坐标
    Vector3f worldPos;    // 世界空间位置
};

// 光照参数
struct LightParams
{
    Vector3f position;    // 光源位置
    Color ambient;        // 环境光颜色
    Color diffuse;        // 漫反射颜色
    Color specular;       // 镜面反射颜色
    float intensity;      // 光照强度
};

// 着色器基类
class Shader
{
public:
    Shader() {}
    virtual ~Shader() {}

    // 顶点着色器接口
    virtual VertexOutput VertexShader(const VertexShaderInput& input) = 0;

    // 片元着色器接口
    virtual Color FragmentShader(const VertexOutput& input, float dudx, float dvdy) = 0;

    // 设置光照参数
    void SetLight(const LightParams& light) { m_light = light; }

protected:
    LightParams m_light;
};

// 简单颜色着色器
class ColorShader : public Shader
{
public:
    ColorShader() {}

    virtual VertexOutput VertexShader(const VertexShaderInput& input) override
    {
        VertexOutput output;
        
        // MVP变换
        Matrix mvp = input.projMatrix * input.viewMatrix * input.modelMatrix;
        output.position = mvp * input.position;
        
        // 计算世界空间位置
        Vector4f worldPos = input.modelMatrix * input.position;
        output.worldPos = Vector3f(worldPos.x, worldPos.y, worldPos.z);
        
        // 计算世界空间法线
        Matrix normalMatrix = input.modelMatrix.transpose().inverse();
        Vector4f worldNormal = normalMatrix * Vector4f(input.normal, 0.0f);
        output.normal = Vector3f(worldNormal.x, worldNormal.y, worldNormal.z).normalize();
        
        // 传递颜色和纹理坐标
        output.color = input.color;
        output.texcoord = input.texcoord;
        
        return output;
    }

    virtual Color FragmentShader(const VertexOutput& input, float dudx, float dvdy) override
    {
        // 直接使用顶点颜色
        return Color(input.color.x, input.color.y, input.color.z, input.color.w);
    }
};

// Phong光照着色器
class PhongShader : public Shader
{
public:
    PhongShader() 
        : m_shininess(32.0f), 
          m_viewPosition(Vector3f(0.0f, 0.0f, 10.0f)) 
    {
        // 默认光照参数
        m_light.position = Vector3f(0.0f, 10.0f, 10.0f);
        m_light.ambient = Color(0.1f, 0.1f, 0.1f, 1.0f);
        m_light.diffuse = Color(0.7f, 0.7f, 0.7f, 1.0f);
        m_light.specular = Color(1.0f, 1.0f, 1.0f, 1.0f);
        m_light.intensity = 1.0f;
    }

    virtual VertexOutput VertexShader(const VertexShaderInput& input) override
    {
        VertexOutput output;
        
        // MVP变换
        Matrix mvp = input.projMatrix * input.viewMatrix * input.modelMatrix;
        output.position = mvp * input.position;
        
        // 计算世界空间位置
        Vector4f worldPos = input.modelMatrix * input.position;
        output.worldPos = Vector3f(worldPos.x, worldPos.y, worldPos.z);
        
        // 计算世界空间法线
        Matrix normalMatrix = input.modelMatrix.transpose().inverse();
        Vector4f worldNormal = normalMatrix * Vector4f(input.normal, 0.0f);
        output.normal = Vector3f(worldNormal.x, worldNormal.y, worldNormal.z).normalize();
        
        // 传递颜色和纹理坐标
        output.color = input.color;
        output.texcoord = input.texcoord;
        
        return output;
    }

    virtual Color FragmentShader(const VertexOutput& input, float dudx, float dvdy) override
    {
        // 基础颜色
        Color baseColor(input.color.x, input.color.y, input.color.z, input.color.w);
        
        // 环境光
        Color ambient = m_light.ambient * baseColor;
        
        // 漫反射
        Vector3f lightDir = (m_light.position - input.worldPos).normalize();
        float diff = (std::max)(0.0f, Vector3f::dot(input.normal, lightDir));
        Color diffuse = m_light.diffuse * baseColor * diff;
        
        // 镜面反射
        Vector3f viewDir = (m_viewPosition - input.worldPos).normalize();
        Vector3f reflectDir = Vector3f::reflect(-lightDir, input.normal);
        float spec = (std::pow)((std::max)(0.0f, Vector3f::dot(viewDir, reflectDir)), m_shininess);
        Color specular = m_light.specular * spec;
        
        // 最终颜色
        Color finalColor = ambient + diffuse + specular;
        finalColor.a = baseColor.a; // 保持原透明度
        
        // 确保颜色分量在[0,1]范围内
        finalColor.r = clamp01(finalColor.r);
        finalColor.g = clamp01(finalColor.g);
        finalColor.b = clamp01(finalColor.b);
        
        return finalColor;
    }

    // 设置观察点位置
    void SetViewPosition(const Vector3f& position) { m_viewPosition = position; }
    
    // 设置高光指数
    void SetShininess(float shininess) { m_shininess = shininess; }

    // 设置光照参数
    void SetLight(const LightParams& light) { m_light = light; }

private:
    float m_shininess;        // 高光指数
    Vector3f m_viewPosition;  // 观察点位置
    LightParams m_light;      // 光照参数
};

// Blinn-Phong光照着色器
class BlinnPhongShader : public Shader
{
public:
    BlinnPhongShader() 
        : m_shininess(32.0f), 
          m_viewPosition(Vector3f(0.0f, 0.0f, 10.0f)) 
    {
        // 默认光照参数
        m_light.position = Vector3f(0.0f, 10.0f, 10.0f);
        m_light.ambient = Color(0.1f, 0.1f, 0.1f, 1.0f);
        m_light.diffuse = Color(0.7f, 0.7f, 0.7f, 1.0f);
        m_light.specular = Color(1.0f, 1.0f, 1.0f, 1.0f);
        m_light.intensity = 1.0f;
    }

    virtual VertexOutput VertexShader(const VertexShaderInput& input) override
    {
        VertexOutput output;
        
        // MVP变换
        Matrix mvp = input.projMatrix * input.viewMatrix * input.modelMatrix;
        output.position = mvp * input.position;
        
        // 计算世界空间位置
        Vector4f worldPos = input.modelMatrix * input.position;
        output.worldPos = Vector3f(worldPos.x, worldPos.y, worldPos.z);
        
        // 计算世界空间法线
        Matrix normalMatrix = input.modelMatrix.transpose().inverse();
        Vector4f worldNormal = normalMatrix * Vector4f(input.normal, 0.0f);
        output.normal = Vector3f(worldNormal.x, worldNormal.y, worldNormal.z).normalize();
        
        // 传递颜色和纹理坐标
        output.color = input.color;
        output.texcoord = input.texcoord;
        
        return output;
    }

    virtual Color FragmentShader(const VertexOutput& input, float dudx, float dvdy) override
    {
        // 基础颜色
        Color baseColor(input.color.x, input.color.y, input.color.z, input.color.w);
        
        // 环境光
        Color ambient = m_light.ambient * baseColor;
        
        // 漫反射
        Vector3f lightDir = (m_light.position - input.worldPos).normalize();
        float diff = (std::max)(0.0f, Vector3f::dot(input.normal, lightDir));
        Color diffuse = m_light.diffuse * baseColor * diff;
        
        // Blinn-Phong高光计算 (不同于Phong的反射计算)
        Vector3f viewDir = (m_viewPosition - input.worldPos).normalize();
        Vector3f halfDir = (lightDir + viewDir).normalize(); // 半程向量
        float spec = (std::pow)((std::max)(0.0f, Vector3f::dot(input.normal, halfDir)), m_shininess);
        Color specular = m_light.specular * spec;
        
        // 最终颜色
        Color finalColor = ambient + diffuse + specular;
        finalColor.a = baseColor.a; // 保持原透明度
        
        // 确保颜色分量在[0,1]范围内
        finalColor.r = clamp01(finalColor.r);
        finalColor.g = clamp01(finalColor.g);
        finalColor.b = clamp01(finalColor.b);
        
        return finalColor;
    }

    // 设置观察点位置
    void SetViewPosition(const Vector3f& position) { m_viewPosition = position; }
    
    // 设置高光指数
    void SetShininess(float shininess) { m_shininess = shininess; }
    
    // 设置光照参数
    void SetLight(const LightParams& light) { m_light = light; }

private:
    float m_shininess;        // 高光指数
    Vector3f m_viewPosition;  // 观察点位置
    LightParams m_light;      // 光照参数
};

// 简单纹理着色器
class TextureShader : public Shader
{
public:
    
    TextureShader() : m_texture(nullptr) {}
    // 设置纹理
    void SetTexture(Texture* texture) { m_texture = texture; }

    virtual VertexOutput VertexShader(const VertexShaderInput& input) override
    {
        VertexOutput output;
        
        // MVP变换
        Matrix mvp = input.projMatrix * input.viewMatrix * input.modelMatrix;
        output.position = mvp * input.position;
        
        // 传递其他属性
        output.color = input.color;
        output.texcoord = input.texcoord;
        output.normal = input.normal;
        
        // 计算世界空间位置
        Vector4f worldPos = input.modelMatrix * input.position;
        output.worldPos = Vector3f(worldPos.x, worldPos.y, worldPos.z);
        
        return output;
    }

    virtual Color FragmentShader(const VertexOutput& input, float dudx, float dvdy) override
    {
        // 采样纹理
        Color texColor = m_texture->Sample(input.texcoord.x, input.texcoord.y, dudx, dvdy);
        
        // 直接返回纹理颜色，不与顶点颜色混合
        return texColor;
    }

private:
    Texture* m_texture;
};

// 纹理 + Blinn-Phong 光照着色器
class TexturedBlinnPhongShader : public Shader
{
public:
    TexturedBlinnPhongShader() 
        : m_texture(nullptr),
          m_shininess(32.0f), 
          m_viewPosition(Vector3f(0.0f, 0.0f, 10.0f)) 
    {
        // 默认光照参数
        m_light.position = Vector3f(0.0f, 10.0f, 10.0f);
        m_light.ambient = Color(0.1f, 0.1f, 0.1f, 1.0f);
        m_light.diffuse = Color(0.7f, 0.7f, 0.7f, 1.0f);
        m_light.specular = Color(1.0f, 1.0f, 1.0f, 1.0f);
        m_light.intensity = 1.0f;
    }

    virtual VertexOutput VertexShader(const VertexShaderInput& input) override
    {
        VertexOutput output;
        
        // MVP变换
        Matrix mvp = input.projMatrix * input.viewMatrix * input.modelMatrix;
        output.position = mvp * input.position;
        
        // 计算世界空间位置
        Vector4f worldPos = input.modelMatrix * input.position;
        output.worldPos = Vector3f(worldPos.x, worldPos.y, worldPos.z);
        
        // 计算世界空间法线
        Matrix normalMatrix = input.modelMatrix.transpose().inverse();
        Vector4f worldNormal = normalMatrix * Vector4f(input.normal, 0.0f);
        output.normal = Vector3f(worldNormal.x, worldNormal.y, worldNormal.z).normalize();
        
        // 传递颜色和纹理坐标
        output.color = input.color;
        output.texcoord = input.texcoord;
        
        return output;
    }

    virtual Color FragmentShader(const VertexOutput& input, float dudx, float dvdy) override
    {
        // 基础颜色 - 从纹理采样或使用顶点颜色
        Color baseColor;
        if (m_texture) {
            // 采样纹理
            Color texColor = m_texture->Sample(input.texcoord.x, input.texcoord.y, dudx, dvdy);
            // 纹理颜色与顶点颜色混合
            // Color vertexColor(input.color.x, input.color.y, input.color.z, input.color.w);
            // baseColor = texColor * vertexColor;

            // 不混合
            baseColor = texColor;
        } else {
            // 无纹理时使用顶点颜色
            baseColor = Color(input.color.x, input.color.y, input.color.z, input.color.w);
        }
        
        // 环境光
        Color ambient = m_light.ambient * baseColor;
        
        // 漫反射
        Vector3f lightDir = (m_light.position - input.worldPos).normalize();
        float diff = (std::max)(0.0f, Vector3f::dot(input.normal, lightDir));
        Color diffuse = m_light.diffuse * baseColor * diff;
        
        // Blinn-Phong高光计算
        Vector3f viewDir = (m_viewPosition - input.worldPos).normalize();
        Vector3f halfDir = (lightDir + viewDir).normalize(); // 半程向量
        float spec = (std::pow)((std::max)(0.0f, Vector3f::dot(input.normal, halfDir)), m_shininess);
        Color specular = m_light.specular * spec;
        
        // 最终颜色
        Color finalColor = ambient + (diffuse + specular) * m_light.intensity;

        finalColor.a = baseColor.a; // 保持原透明度
        
        // 确保颜色分量在[0,1]范围内
        finalColor.r = clamp01(finalColor.r);
        finalColor.g = clamp01(finalColor.g);
        finalColor.b = clamp01(finalColor.b);
        
        return finalColor;
    }

    // 设置纹理
    void SetTexture(Texture* texture) { m_texture = texture; }
    
    // 设置观察点位置
    void SetViewPosition(const Vector3f& position) { m_viewPosition = position; }
    
    // 设置高光指数
    void SetShininess(float shininess) { m_shininess = shininess; }
    
    // 设置光照参数
    void SetLight(const LightParams& light) { m_light = light; }

private:
    Texture* m_texture;       // 纹理
    float m_shininess;        // 高光指数
    Vector3f m_viewPosition;  // 观察点位置
    LightParams m_light;      // 光照参数
}; 