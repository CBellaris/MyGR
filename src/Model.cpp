#include "Model.h"
#include "Mesh.h"
#include "Renderer.h"

Shader* Model::get_singleColor_shader()
{
    static Shader* singleColorShader = nullptr;
    if (!singleColorShader)
    {
        singleColorShader = new Shader("res/shader/Single_color.shader");
    }
    return singleColorShader;
}

void Model::add_basic_geom(BasicGeom geomType, Texture *texture)
{
    Mesh* mesh_ = new Mesh();
    switch (geomType)
    {
    case BasicGeom::Screen:{
        mesh_->set_mesh_screen();
        if(texture)
            mesh_->set_texture(texture);
        meshes.push_back(mesh_); 
        break;
    } 
    case BasicGeom::Plane:{
        mesh_->set_mesh_plane();
        if(texture)
            mesh_->set_texture(texture);
        meshes.push_back(mesh_); 
        break;
    }       
    case BasicGeom::Cube:{
        mesh_->set_mesh_cube();
        if(texture)
            mesh_->set_texture(texture);
        meshes.push_back(mesh_);    
        break;
    }
    case BasicGeom::Sphere:{
        mesh_->set_mesh_sphere(32, 18, 1.0f); // 32个经线，18个纬线，半径1.0
        if(texture)
            mesh_->set_texture(texture);
        meshes.push_back(mesh_); 
        break;
    }
    default:
        break;
    }
}

// 依次绘制所有网格
void Model::draw(Shader* shader)
{
    
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i]->draw(shader);

    draw_outline();
}

void Model::draw_outline(Shader* outlineShader)
{
    if (!ifDrawOutline) return;
    if (!outlineShader) outlineShader = get_singleColor_shader();

    glEnable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST); 
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // 让模板缓冲区无论深度测试结果如何都被写入
    glClear(GL_STENCIL_BUFFER_BIT);

    // 1. 第一遍绘制：写入模板值 1，但不写入颜色
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // 禁止颜色写入

    outlineShader->bind();
    for (auto& mesh : meshes)
        mesh->draw(outlineShader);

    // 2. 第二遍绘制：放大模型，绘制轮廓
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // 允许颜色写入

    outlineShader->setUniform3f("outlineColor", outlineColor.x, outlineColor.y, outlineColor.z);

    for (auto& mesh : meshes) {
        mesh->set_scale(glm::vec3(1.03f));
        mesh->draw(outlineShader);
        mesh->set_scale(glm::vec3(1.0f));
    }

    // 3. 关闭模板测试，恢复默认状态
    glStencilMask(0xFF); // 注意这里，如果设置glStencilMask(0x00);那么glClear(GL_STENCIL_BUFFER_BIT);同样将不起作用，所以在最后我们恢复模板的写入
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST); 
}




void Model::loadModel(std::string path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs| aiProcess_CalcTangentSpace);    

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    // 处理节点所有的网格（如果有的话）
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        meshes.push_back(processMesh(mesh, scene));         
    }
    // 接下来对它的子节点重复这一过程
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh* Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertexdata> vertices;
    std::vector<unsigned int> indices;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertexdata vertex;
        // 处理顶点位置、法线、纹理坐标和切线及副切线
        glm::vec3 vector; 
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z; 
        vertex.Position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;

        vector.x = mesh->mTangents[i].x;
        vector.y = mesh->mTangents[i].y;
        vector.z = mesh->mTangents[i].z;
        vertex.Tangent = vector;

        if(mesh->mTextureCoords[0]) // 网格是否有纹理坐标？
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x; 
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    // 处理索引
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }


    // 处理材质
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex]; 

    Texture* texture = new Texture();
    if (mesh->mMaterialIndex >= 0) {
        process_material_textures(material, aiTextureType_DIFFUSE, TextureType::Diffuse, texture, scene, this->directory); 
        process_material_textures(material, aiTextureType_METALNESS, TextureType::Metallic, texture, scene, this->directory);  
        process_material_textures(material, aiTextureType_DIFFUSE_ROUGHNESS, TextureType::Roughness, texture, scene, this->directory);  
        process_material_textures(material, aiTextureType_HEIGHT, TextureType::Height, texture, scene, this->directory);
        process_material_textures(material, aiTextureType_NORMALS, TextureType::Normal, texture, scene, this->directory);
        process_material_textures(material, aiTextureType_AMBIENT, TextureType::AO, texture, scene, this->directory);
    }

    Mesh* mesh_ = new Mesh();
    mesh_->set_mesh(vertices, indices);
    mesh_->set_texture(texture);
    
    return mesh_;
}

// 目前一共有三种贴图加载方式，分别是：
// 1. 嵌入贴图-压缩图片 (e.g., PNG/JPG)（Assimp会将纹理数据嵌入到模型文件中）
// 2. 嵌入贴图-非压缩图像数据（像素数组）（Assimp会将纹理数据嵌入到模型文件中）
// 3. 外部贴图（Assimp会将纹理数据存储在外部文件中）
void Model::process_material_textures(
    aiMaterial* material,
    aiTextureType assimpType,
    TextureType myType,
    Texture* texture,
    const aiScene* scene,
    const std::string& directory
) {
    unsigned int texCount = material->GetTextureCount(assimpType);

    for (unsigned int i = 0; i < texCount; ++i) {
        aiString str;
        material->GetTexture(assimpType, i, &str);
        std::string texPath = str.C_Str();

        const aiTexture* embeddedTex = nullptr;

        // Assimp 样式内嵌贴图路径 "*0"
        if (!texPath.empty() && texPath[0] == '*') {
            int index = std::stoi(texPath.substr(1));
            if (index >= 0 && index < static_cast<int>(scene->mNumTextures)) {
                embeddedTex = scene->mTextures[index];
            }
        }

        // FBX 虚拟路径，如 "scene.fbm/Tex_0001.png"
        if (!embeddedTex) {
            for (unsigned int t = 0; t < scene->mNumTextures; ++t) {
                if (scene->mTextures[t]->mFilename.C_Str() == texPath) {
                    embeddedTex = scene->mTextures[t];
                    break;
                }
            }
        }

        if (embeddedTex) {
            // 嵌入贴图处理
            if (embeddedTex->mHeight == 0) {
                // 压缩图像（PNG/JPG）
                const unsigned char* data = reinterpret_cast<unsigned char*>(embeddedTex->pcData);
                size_t size = embeddedTex->mWidth;
                texture->add_image(texPath, myType, data, size);
            } else {
                // 原始像素图（一般是 BGRA 格式）
                const unsigned char* raw = reinterpret_cast<unsigned char*>(embeddedTex->pcData);
                int width = embeddedTex->mWidth;
                int height = embeddedTex->mHeight;
                int channels = 4; // Assimp 默认是 BGRA，每像素 4 字节
                texture->add_image_from_raw(texPath, myType, raw, width, height, channels);
            }
        } else {
            // 外部贴图处理
            std::string filepath = directory + '/' + texPath;
            texture->add_image(filepath, myType);
        }
    }
}
