#pragma once

#include <string>
#include <vector>
#include "Object.h"
#include "Vector.h"

class ObjFileReader {
public:
    ObjFileReader();
    ~ObjFileReader();

    // 从文件加载OBJ模型，返回构建好的Object对象
    static Object LoadFromFile(const std::string& filePath);
    
    // 从文件加载OBJ模型，可以指定是否翻转法线和面片环绕顺序
    static Object LoadFromFileWithOptions(const std::string& filePath, 
                                         bool flipNormals = false, 
                                         bool flipFaces = false);
    
    // 从文件加载OBJ模型，并构建Mesh
    static Mesh LoadMeshFromFile(const std::string& filePath);
    
    // 从文件加载OBJ模型，可以指定是否翻转法线和面片环绕顺序
    static Mesh LoadMeshFromFileWithOptions(const std::string& filePath, 
                                          bool flipNormals = false, 
                                          bool flipFaces = false);
    
private:
    // 解析OBJ文件中的一行数据
    static void ParseLine(const std::string& line, 
                   std::vector<Vector3f>& positions,
                   std::vector<Vector2f>& texcoords,
                   std::vector<Vector3f>& normals,
                   std::vector<int>& positionIndices,
                   std::vector<int>& texcoordIndices,
                   std::vector<int>& normalIndices);
    
    // 从已解析的数据构建Mesh
    static Mesh BuildMesh(const std::vector<Vector3f>& positions,
                   const std::vector<Vector2f>& texcoords,
                   const std::vector<Vector3f>& normals,
                   const std::vector<int>& positionIndices,
                   const std::vector<int>& texcoordIndices,
                   const std::vector<int>& normalIndices,
                   bool flipNormals = false,
                   bool flipFaces = false);
};
