#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp> // 实验功能，使用了glm::toMat4()

#include "BufferObject.h"
#include "Texture.h"


struct Vertexdata
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    Vertexdata(float px, float py, float pz,
               float nx, float ny, float nz,
               float tx, float ty): Position(px, py, pz), Normal(nx, ny, nz), TexCoords(tx, ty) {}
};

class Mesh
{
private:
    std::vector<Vertexdata> vertices;
    std::vector<unsigned int> indices;
    Texture* textures;
    VertexArrayObject* VAO;

    glm::mat4 model;
    glm::vec3 position;
    glm::vec3 eulerAngles; // 存储用户设置的欧拉角（旋转角度）
    glm::vec3 scale;

public:
    Mesh();
    ~Mesh();

    void setupMesh(std::vector<Vertexdata> vertices, std::vector<unsigned int> indices);
    //void addTexture();

    void setupMeshCube();
    void bind();
    void unbind();
    inline int getNumElements() const{
        return indices.size();
    }

    // 设置模型的位移
    void setPosition(const glm::vec3& pos) ;
    // 设置模型的旋转（使用角度）
    void setRotation(const glm::vec3& angles);
    // 设置模型的缩放
    void setScale(const glm::vec3& scl);

    // 获取当前的模型矩阵
    inline const glm::mat4& getModelMatrix() const {
        return model;
    }

private:
    // 更新模型矩阵
    void updateModelMatrix();
 
};