#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp> // 实验功能，使用了glm::toMat4()

#include "BufferObject.h"
#include "Texture.h"

class Shader;

struct Vertexdata
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    glm::vec3 Tangent;   // 切线
    float tangentW;      // 切线方向标志位（+1 或 -1）

    Vertexdata(float px = 0.0f, float py = 0.0f, float pz = 0.0f,
               float nx = 0.0f, float ny = 0.0f, float nz = 0.0f,
               float tx = 0.0f, float ty = 0.0f)
               : Position(px, py, pz), Normal(nx, ny, nz), TexCoords(tx, ty), 
               Tangent(0.0f), tangentW(1.0f) {}
};

class Mesh
{
private:
    std::vector<Vertexdata> vertices;
    std::vector<unsigned int> indices;
    Texture* texture;
    VertexArrayObject* VAO;

    glm::mat4 model;
    glm::vec3 position;
    glm::vec3 eulerAngles; // 存储用户设置的欧拉角（旋转角度）
    glm::vec3 scale;

    // 可视性
    bool visibility;

public:
    Mesh();
    ~Mesh();

    void set_mesh(std::vector<Vertexdata> vertices, std::vector<unsigned int> indices, bool if_Cal_Tangents = false);
    void set_texture(Texture* texture);

    void set_mesh_screen();
    void set_mesh_plane();
    void set_mesh_cube();
    void set_mesh_sphere(int sectorCount, int stackCount, float radius = 1.0f);

    inline int getNumElements() const{
        return indices.size();
    }

    // 设置模型的位移
    void set_position(const glm::vec3& pos) ;
    // 设置模型的旋转（使用角度）
    void set_rotation(const glm::vec3& angles);
    // 设置模型的缩放
    void set_scale(const glm::vec3& scl);

    // 获取接口
    inline const glm::vec3& get_position(){return position;}

    // 获取当前的模型矩阵
    inline const glm::mat4& getModelMatrix() const {
        return model;
    }

    // 设置可视性
    void set_visibility(const bool visiable);
    inline bool get_visibility(){return visibility;}

    void draw(Shader* shader = nullptr);

private:
    // 更新模型矩阵
    void updateModelMatrix();

    // 切线和副切线计算函数
    void calculateTangents(
        const std::vector<Vertexdata>& vertices,
        const std::vector<unsigned int>& indices,
        std::vector<glm::vec3>& tangents);
    
    void bind();
    void unbind();
};