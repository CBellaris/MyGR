#include "Mesh.h"

Mesh::Mesh(): model(glm::mat4(1.0f)), position(0.0f), eulerAngles(0.0f), scale(1.0f)
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

void Mesh::setPosition(const glm::vec3 &pos)
{
    position = pos;
    updateModelMatrix();
}

void Mesh::setRotation(const glm::vec3& angles)
{
    eulerAngles = angles;
    updateModelMatrix();
}

void Mesh::setScale(const glm::vec3& scl) 
{
    scale = scl;
    updateModelMatrix();
}

void Mesh::render()
{
    glDrawElements(GL_TRIANGLES, getNumElements(), GL_UNSIGNED_INT, 0);
}

void Mesh::updateModelMatrix()
{
    model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    glm::quat rotationQuat = glm::quat(glm::radians(eulerAngles)); // 将角度转换为弧度
    model *= glm::toMat4(rotationQuat); // 使用四元数生成旋转矩阵
    model = glm::scale(model, scale);
}
