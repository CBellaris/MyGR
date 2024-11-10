#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
    delete textures;
    delete VAO;
}

void Mesh::setupMesh(std::vector<Vertexdata> vertices, std::vector<unsigned int> indices)
{
    this->vertices = vertices;
    this->indices = indices;
    // 使用自定义的类，管理一个VAO，并绑定VBO和IBO
    VAO = new VertexArrayObject();
    VAO->addVertexBuffer(vertices);
    VAO->addIndexBuffer(indices);
    VAO->push<float>(3);
    VAO->push<float>(3);
    VAO->push<float>(2);
    VAO->bindAll();
}

void Mesh::setupMeshCube()
{
    std::vector<Vertexdata> verticesCube = {
    Vertexdata(-0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f),
    Vertexdata( 0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, 1.0f, 0.0f),
    Vertexdata( 0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, 1.0f, 1.0f),
    Vertexdata( 0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, 1.0f, 1.0f),
    Vertexdata(-0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, 0.0f, 1.0f),
    Vertexdata(-0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f),

    Vertexdata(-0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, 0.0f, 0.0f),
    Vertexdata( 0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, 1.0f, 0.0f),
    Vertexdata( 0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, 1.0f, 1.0f),
    Vertexdata( 0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, 1.0f, 1.0f),
    Vertexdata(-0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, 0.0f, 1.0f),
    Vertexdata(-0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, 0.0f, 0.0f),

    Vertexdata(-0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, 1.0f, 0.0f),
    Vertexdata(-0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, 1.0f, 1.0f),
    Vertexdata(-0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f),
    Vertexdata(-0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f),
    Vertexdata(-0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, 0.0f, 0.0f),
    Vertexdata(-0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, 1.0f, 0.0f),

    Vertexdata( 0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, 1.0f, 0.0f),
    Vertexdata( 0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, 1.0f, 1.0f),
    Vertexdata( 0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, 0.0f, 1.0f),
    Vertexdata( 0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, 0.0f, 1.0f),
    Vertexdata( 0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, 0.0f, 0.0f),
    Vertexdata( 0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, 1.0f, 0.0f),

    Vertexdata(-0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f),
    Vertexdata( 0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, 1.0f, 1.0f),
    Vertexdata( 0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, 1.0f, 0.0f),
    Vertexdata( 0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, 1.0f, 0.0f),
    Vertexdata(-0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, 0.0f, 0.0f),
    Vertexdata(-0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f),

    Vertexdata(-0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, 0.0f, 1.0f),
    Vertexdata( 0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, 1.0f, 1.0f),
    Vertexdata( 0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, 1.0f, 0.0f),
    Vertexdata( 0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, 1.0f, 0.0f),
    Vertexdata(-0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, 0.0f, 0.0f),
    Vertexdata(-0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, 0.0f, 1.0f)
    };

    std::vector<unsigned int> indicesDummy;
    for (unsigned int i=0; i<verticesCube.size(); i++)
        indicesDummy.push_back(i);
    setupMesh(verticesCube, indicesDummy);
}

void Mesh::bind()
{
    VAO->bind();
}

void Mesh::unbind()
{
    VAO->unbind();
}

