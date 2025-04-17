#include "../include/ObjFileReader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

ObjFileReader::ObjFileReader() {
}

ObjFileReader::~ObjFileReader() {
}

Object ObjFileReader::LoadFromFile(const std::string& filePath) {
    // 使用默认选项（不翻转法线和面片）
    return LoadFromFileWithOptions(filePath, false, false);
}

Object ObjFileReader::LoadFromFileWithOptions(const std::string& filePath, bool flipNormals, bool flipFaces) {
    // 加载Mesh，可以指定是否翻转法线和面片
    Mesh mesh = LoadMeshFromFileWithOptions(filePath, flipNormals, flipFaces);
    
    // 创建默认材质和变换
    Material material;
    Transformer transformer;
    
    // 构建并返回Object
    return Object(mesh, material, transformer);
}

Mesh ObjFileReader::LoadMeshFromFile(const std::string& filePath) {
    // 使用默认选项（不翻转法线和面片）
    return LoadMeshFromFileWithOptions(filePath, false, false);
}

Mesh ObjFileReader::LoadMeshFromFileWithOptions(const std::string& filePath, bool flipNormals, bool flipFaces) {
    // 存储顶点数据
    std::vector<Vector3f> positions;    // v
    std::vector<Vector2f> texcoords;    // vt
    std::vector<Vector3f> normals;      // vn
    
    // 存储索引数据
    std::vector<int> positionIndices;   // f v1/vt1/vn1 的v1部分
    std::vector<int> texcoordIndices;   // f v1/vt1/vn1 的vt1部分
    std::vector<int> normalIndices;     // f v1/vt1/vn1 的vn1部分
    
    // 打开文件
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << filePath << std::endl;
        return Mesh(); // 返回空Mesh
    }
    
    // 逐行读取文件
    std::string line;
    while (std::getline(file, line)) {
        ParseLine(line, positions, texcoords, normals, 
                 positionIndices, texcoordIndices, normalIndices);
    }
    
    // 关闭文件
    file.close();
    
    // 从数据构建并返回Mesh，可以指定是否翻转法线和面片
    return BuildMesh(positions, texcoords, normals, 
                    positionIndices, texcoordIndices, normalIndices,
                    flipNormals, flipFaces);
}

void ObjFileReader::ParseLine(const std::string& line, 
                            std::vector<Vector3f>& positions,
                            std::vector<Vector2f>& texcoords,
                            std::vector<Vector3f>& normals,
                            std::vector<int>& positionIndices,
                            std::vector<int>& texcoordIndices,
                            std::vector<int>& normalIndices) {
    // 跳过空行和注释行
    if (line.empty() || line[0] == '#')
        return;
    
    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix;
    
    if (prefix == "g" || prefix == "usemtl" || prefix == "mtllib" || prefix == "s") {
        // 忽略这些命令
        return;
    }
    else if (prefix == "v") {
        // 顶点位置 (v x y z)
        float x, y, z;
        iss >> x >> y >> z;
        positions.push_back(Vector3f(x, y, z));
    }
    else if (prefix == "vt") {
        // 纹理坐标 (vt u v)
        float u, v;
        iss >> u >> v;
        // 翻转V坐标以匹配DirectX/OpenGL纹理坐标系统
        texcoords.push_back(Vector2f(u, 1.0f - v));
    }
    else if (prefix == "vn") {
        // 法线 (vn x y z)
        float x, y, z;
        iss >> x >> y >> z;
        normals.push_back(Vector3f(x, y, z));
    }
    else if (prefix == "f") {
        // 面 (f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3)
        std::string vertexData;
        std::vector<int> facePositionIndices;
        std::vector<int> faceTexcoordIndices;
        std::vector<int> faceNormalIndices;
        
        // 读取每个顶点的数据
        while (iss >> vertexData) {
            // 检查顶点数据的格式
            size_t slashCount = std::count(vertexData.begin(), vertexData.end(), '/');
            
            // 解析顶点数据
            std::istringstream vertexIss(vertexData);
            std::string token;
            int pi = 0, ti = 0, ni = 0;
            bool hasTexcoord = false;
            bool hasNormal = false;
            
            // 读取位置索引（必须有）
            if (std::getline(vertexIss, token, '/')) {
                if (!token.empty()) {
                    pi = std::stoi(token);
                    if (pi < 0) { // 处理负索引（相对于当前位置的偏移）
                        pi = positions.size() + pi + 1;
                    }
                    facePositionIndices.push_back(pi - 1); // OBJ索引从1开始
                }
            }
            
            // 读取纹理坐标索引（可选）
            if (slashCount > 0 && std::getline(vertexIss, token, '/')) {
                if (!token.empty()) {
                    ti = std::stoi(token);
                    if (ti < 0) { // 处理负索引
                        ti = texcoords.size() + ti + 1;
                    }
                    hasTexcoord = true;
                }
            }
            
            // 读取法线索引（可选）
            if (slashCount > 1 && std::getline(vertexIss, token, '/')) {
                if (!token.empty()) {
                    ni = std::stoi(token);
                    if (ni < 0) { // 处理负索引
                        ni = normals.size() + ni + 1;
                    }
                    hasNormal = true;
                }
            }
            
            // 添加纹理坐标索引
            if (hasTexcoord && ti > 0 && ti <= texcoords.size()) {
                faceTexcoordIndices.push_back(ti - 1);
            } else {
                // 如果没有纹理坐标，使用顶点的相对位置作为UV
                Vector3f pos = positions[facePositionIndices.back()];
                float u = (pos.x + 1.0f) * 0.5f; // 将x坐标映射到[0,1]
                float v = (pos.y + 1.0f) * 0.5f; // 将y坐标映射到[0,1]
                texcoords.push_back(Vector2f(u, v));
                faceTexcoordIndices.push_back(texcoords.size() - 1);
            }
            
            // 添加法线索引
            if (hasNormal && ni > 0 && ni <= normals.size()) {
                faceNormalIndices.push_back(ni - 1);
            } else {
                faceNormalIndices.push_back(0); // 使用默认法线
            }
        }
        
        // 处理四边形面片（转换为两个三角形）
        if (facePositionIndices.size() == 4) {
            // 第一个三角形
            positionIndices.push_back(facePositionIndices[0]);
            positionIndices.push_back(facePositionIndices[1]);
            positionIndices.push_back(facePositionIndices[2]);
            
            texcoordIndices.push_back(faceTexcoordIndices[0]);
            texcoordIndices.push_back(faceTexcoordIndices[1]);
            texcoordIndices.push_back(faceTexcoordIndices[2]);
            
            normalIndices.push_back(faceNormalIndices[0]);
            normalIndices.push_back(faceNormalIndices[1]);
            normalIndices.push_back(faceNormalIndices[2]);
            
            // 第二个三角形
            positionIndices.push_back(facePositionIndices[0]);
            positionIndices.push_back(facePositionIndices[2]);
            positionIndices.push_back(facePositionIndices[3]);
            
            texcoordIndices.push_back(faceTexcoordIndices[0]);
            texcoordIndices.push_back(faceTexcoordIndices[2]);
            texcoordIndices.push_back(faceTexcoordIndices[3]);
            
            normalIndices.push_back(faceNormalIndices[0]);
            normalIndices.push_back(faceNormalIndices[2]);
            normalIndices.push_back(faceNormalIndices[3]);
        }
        // 处理三角形面片
        else if (facePositionIndices.size() == 3) {
            positionIndices.insert(positionIndices.end(), facePositionIndices.begin(), facePositionIndices.end());
            texcoordIndices.insert(texcoordIndices.end(), faceTexcoordIndices.begin(), faceTexcoordIndices.end());
            normalIndices.insert(normalIndices.end(), faceNormalIndices.begin(), faceNormalIndices.end());
        }
    }
}

Mesh ObjFileReader::BuildMesh(const std::vector<Vector3f>& positions,
                            const std::vector<Vector2f>& texcoords,
                            const std::vector<Vector3f>& normals,
                            const std::vector<int>& positionIndices,
                            const std::vector<int>& texcoordIndices,
                            const std::vector<int>& normalIndices,
                            bool flipNormals,
                            bool flipFaces) {
    // 创建新的Mesh
    Mesh mesh;
    
    // 确保有顶点数据
    if (positions.empty() || positionIndices.empty()) {
        std::cerr << "OBJ file does not contain valid vertex data" << std::endl;
        return mesh;
    }
    
    // 确保有有效的纹理坐标
    bool hasTexcoords = !texcoords.empty() && !texcoordIndices.empty() && 
                       texcoords.size() > 0 && texcoordIndices.size() >= positionIndices.size();
    
    // 确保有有效的法线
    bool hasNormals = !normals.empty() && !normalIndices.empty() && 
                     normals.size() > 0 && normalIndices.size() >= positionIndices.size();
    
    // 添加所有顶点
    for (size_t i = 0; i < positionIndices.size(); i += 3) {
        // 确保有足够的索引来构成一个三角形
        if (i + 2 >= positionIndices.size()) {
            break;
        }
        
        // 决定顶点的添加顺序（是否翻转面片）
        int order[3] = {0, 1, 2}; // 默认顺序
        if (flipFaces) {
            // 翻转顶点顺序，使得三角形的环绕方向反转
            order[0] = 0;
            order[1] = 2;
            order[2] = 1;
        }
        
        // 为三角形添加三个顶点（可能按照翻转后的顺序）
        for (int j = 0; j < 3; ++j) {
            int vertexIndex = order[j]; // 使用可能翻转后的索引顺序
            
            // 获取当前点的索引
            int pIdx = positionIndices[i + vertexIndex];
            int tIdx = hasTexcoords ? texcoordIndices[i + vertexIndex] : 0;
            int nIdx = hasNormals ? normalIndices[i + vertexIndex] : 0;
            
            // 获取位置（必须有）
            Vector3f position = positions[pIdx];
            
            // 获取纹理坐标（如果有）
            Vector2f texcoord = hasTexcoords && tIdx < texcoords.size() ? 
                               texcoords[tIdx] : Vector2f(0.0f, 0.0f);
            
            // 获取法线（如果有），可能需要翻转
            Vector3f normal = hasNormals && nIdx < normals.size() ? 
                             normals[nIdx] : Vector3f(0.0f, 1.0f, 0.0f);
            
            // 如果需要，翻转法线
            if (flipNormals) {
                normal = -normal;
            }
            
            // 创建顶点并添加到Mesh
            Vertex vertex(
                Vector4f(position.x, position.y, position.z, 1.0f),
                Vector4f(1.0f, 1.0f, 1.0f, 1.0f), // 默认白色
                normal,
                texcoord
            );
            
            mesh.AddVertex(vertex);
        }
        
        // 添加当前三角形的索引
        int baseIndex = mesh.vertices.size() - 3;
        mesh.AddTriangle(baseIndex, baseIndex + 1, baseIndex + 2);
    }
    
    // 如果没有法线，计算面法线
    if (!hasNormals) {
        for (size_t i = 0; i < mesh.indices.size(); ++i) {
            const Vector3i& index = mesh.indices[i];
            
            // 获取三角形的三个顶点
            Vector3f v0 = Vector3f(
                mesh.vertices[index.x].pos.x, 
                mesh.vertices[index.x].pos.y, 
                mesh.vertices[index.x].pos.z
            );
            Vector3f v1 = Vector3f(
                mesh.vertices[index.y].pos.x, 
                mesh.vertices[index.y].pos.y, 
                mesh.vertices[index.y].pos.z
            );
            Vector3f v2 = Vector3f(
                mesh.vertices[index.z].pos.x, 
                mesh.vertices[index.z].pos.y, 
                mesh.vertices[index.z].pos.z
            );
            
            // 计算面法线
            Vector3f edge1 = v1 - v0;
            Vector3f edge2 = v2 - v0;
            Vector3f normal = Vector3f::cross(edge1, edge2).normalize();
            
            // 如果需要，翻转法线
            if (flipNormals) {
                normal = -normal;
            }
            
            // 设置三角形每个顶点的法线
            mesh.vertices[index.x].normal = normal;
            mesh.vertices[index.y].normal = normal;
            mesh.vertices[index.z].normal = normal;
        }
    }
    
    std::cout << "OBJ Loaded: " << mesh.vertices.size() << " vertices, " 
              << mesh.indices.size() << " triangles" 
              << (flipNormals ? " (normals flipped)" : "")
              << (flipFaces ? " (faces flipped)" : "")
              << std::endl;
    
    return mesh;
}
