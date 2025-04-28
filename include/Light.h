#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Model.h"
#include "Shader.h"
#include "GlobalSettings.h"
#include "BufferObject.h"
#include <string>

// 灯光基础信息
struct LightUnit {
    alignas(16) glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);  
    alignas(16) glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    alignas(16) glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);     
    alignas(16) glm::vec3 intensity = glm::vec3(0.2f, 0.5f, 0.5f);  // 分量分别为ambient, diffuse和specular的强度


    alignas(4) float constant = 1.0f;
    alignas(4) float linear = 	0.14f;
    alignas(4) float quadratic = 0.07f;

    // 默认可见并且为点光
    alignas(4) int visibility = 1;
    alignas(4) int isDirectional = 0;

    alignas(4) float padding1;
    alignas(4) float padding2;
    alignas(4) float padding3;
};

// 灯光的阴影贴图相关信息
struct ShadowMapInfo {
    bool isDirectional;  // true：平行光使用2D阴影；false：点光使用立方体阴影
    unsigned int fbo;    // 帧缓冲对象
    unsigned int texture; // 阴影贴图纹理ID，平行光为2D纹理，点光为立方体纹理
};

class Light
{
private:
    std::vector<LightUnit> lights;
    const int MAX_SHADOW_MAP_SLOTS = GlobalSettings::getInstance().GetInt("MAX_SHADOW_MAP_TEXTURE_SLOTS");
    int directionalLightCount = 0;
    int pointLightCount = 0;
    std::vector<ShadowMapInfo> shadowMaps; // 每个灯光对应一个阴影贴图信息

    // 阴影贴图尺寸
    const unsigned int SHADOW_RATE = 1;  // 屏幕的SHADOW_RATE倍分辨率
    const unsigned int CUBE_SHADOW_SIZE = 1024;

    // 平行光的投影矩阵
    const float orthoSize = 15.0f;
    glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 100.0f);
    // 点阴影远裁剪面
    float far_plane = 20.0f;


public:
    Light();
    ~Light();

    SSBO<LightUnit>* lightsSSBO;
    Mesh* lightCube;

    // 添加灯光
    void add_light(LightUnit lightUnit);

    // 设置灯光的SSBO
    void set_sUniform_light(Shader* shader);
    void draw(Shader* shader);

    // 烘焙阴影贴图
    void bakeShadows(Shader* shadowMapShader_directionalLight, Shader* shadowMapShader_pointLight, std::vector<std::shared_ptr<Model>>& models);

    // 绑定使用阴影贴图的着色器
    void bind_shadow(Shader* shader);

    // TEST
    inline unsigned int get_textureID(int index) {return shadowMaps[index].texture;}

    static glm::vec3 hexToVec3(const std::string& hexStr);

private:
    // 根据灯光类型初始化阴影贴图
    void initDirectionalShadowMap(ShadowMapInfo &shadowInfo);
    void initPointShadowMap(ShadowMapInfo &shadowInfo);
};