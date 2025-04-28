#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include <iostream>

#include "Renderer.h"

class Shader;
class Texture;
class Mesh;

enum class BasicGeom{
    Screen,
    Plane,
    Cube,
    Sphere
};


class Model 
{
    public:

        Model(const std::string& filepath = ""): outlineColor(glm::vec3(245/255.0f, 203/255.0f, 111/255.0f)), ifDrawOutline(false)
        {
            if(!filepath.empty())
                loadModel(filepath);
        }
        ~Model() {
            for (auto mesh : meshes)
                delete mesh;
        }

        void add_basic_geom(BasicGeom geomType, Texture* texture = nullptr);

        inline void set_ifDrawOutline(bool ifDrawOutline) {this->ifDrawOutline = ifDrawOutline;}
        inline void set_position(const glm::vec3& pos) {for(auto mesh : meshes) mesh->set_position(pos);}
        inline void set_scale(const glm::vec3& scl) {for(auto mesh : meshes) mesh->set_scale(scl);}
        inline void set_rotation(const glm::vec3& rot) {for(auto mesh : meshes) mesh->set_rotation(rot);}

        void draw(Shader* shader);   
        void draw_outline(Shader* outlineShader = nullptr);
    private:
        std::vector<Mesh*> meshes;
        std::string directory;

        // 绘制轮廓
        Shader* get_singleColor_shader();  // 懒加载，推迟实例化到需要使用时，以免在OpenGL上下文被创建之前编译着色器程序
        bool ifDrawOutline;
        glm::vec3 outlineColor;

        void loadModel(std::string path);
        void processNode(aiNode *node, const aiScene *scene);
        Mesh* processMesh(aiMesh *mesh, const aiScene *scene);

        void process_material_textures(
            aiMaterial* material,
            aiTextureType assimpType,
            TextureType myType,
            Texture* texture,
            const aiScene* scene,
            const std::string& directory
        );
};