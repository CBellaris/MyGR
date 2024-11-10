#pragma once
#include <vector>
#include <glm/glm.hpp>

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

    glm::mat4 scale;


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
 
};