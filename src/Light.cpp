#include "Light.h"

#include "Renderer.h"

Light::Light()
{
    lightCube = new Mesh();
    lightCube->set_mesh_cube();
    lightCube->set_scale(glm::vec3(0.1f)); // 设置一个小立方体用于标示灯光

    lightsSSBO = new SSBO<LightUnit>(1, 10);
}

Light::~Light()
{
    delete lightCube;
    delete lightsSSBO;
    // 删除每个阴影贴图的FBO和纹理
    for(auto &shadow : shadowMaps) {
        glDeleteFramebuffers(1, &shadow.fbo);
        glDeleteTextures(1, &shadow.texture);
    }
}


void Light::add_light(LightUnit lightUnit)
{
    // 如果为平行光，将灯光位置设在远处以便观察
    if(lightUnit.isDirectional == 1){
        directionalLightCount ++;
        if(directionalLightCount>MAX_SHADOW_MAP_SLOTS){
            std::cout<<"Warning: directional light exceed the max slots"<<std::endl;
            return;
        }
        float back_dist = 10.0f;
        lightUnit.position = -back_dist * lightUnit.direction;
    }else{
        pointLightCount ++;
        if(pointLightCount>MAX_SHADOW_MAP_SLOTS){
            std::cout<<"Warning: point light exceed the max slots"<<std::endl;
            return;
        }
    }

    lights.push_back(lightUnit);
    lightsSSBO->updateData(lights);

    // 根据灯光类型初始化对应的阴影贴图
    ShadowMapInfo shadowInfo;
    shadowInfo.isDirectional = (lightUnit.isDirectional == 1);
    if(shadowInfo.isDirectional){
        initDirectionalShadowMap(shadowInfo);
    } else {
        initPointShadowMap(shadowInfo);
    }
    shadowMaps.push_back(shadowInfo);
}

void Light::set_sUniform_light(Shader *shader)
{
    shader->setUniform1i("numLights", static_cast<int>(lights.size()));
}

void Light::draw(Shader* shader)
{
    for (const auto& light : lights){
        if(light.visibility){
            shader->setUniform3f("lightColor", light.color.x, light.color.y, light.color.z);
            lightCube->set_position(light.position);
            lightCube->draw(shader); // 渲染小立方体
        }
    }
}


void Light::bakeShadows(Shader* shadowMapShader_directionalLight, Shader* shadowMapShader_pointLight, std::vector<std::shared_ptr<Model>>& models)
{
    for (size_t i = 0; i < lights.size(); i++) {
        LightUnit &light = lights[i];
        ShadowMapInfo &shadow = shadowMaps[i];

        if(shadow.isDirectional) {
            // 平行光使用正交投影
            glm::mat4 lightView = glm::lookAt(light.position, light.position + light.direction, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 lightSpaceMatrix = lightProjection * lightView;
            shadowMapShader_directionalLight->bind();
            shadowMapShader_directionalLight->setUniform4fv("lightSpaceMatrix", lightSpaceMatrix);

            // 渲染阴影贴图
            glViewport(0, 0, GlobalSettings::getInstance().GetInt("SCREEN_WIDTH")* SHADOW_RATE,GlobalSettings::getInstance().GetInt("SCREEN_HEIGHT")* SHADOW_RATE);
            glBindFramebuffer(GL_FRAMEBUFFER, shadow.fbo);
            glClear(GL_DEPTH_BUFFER_BIT);
            for(auto model : models) model->draw(shadowMapShader_directionalLight);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        } else {
            // 点光使用立方体贴图：构造6个视角
            float near_plane = 1.0f;
            glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, near_plane, far_plane);
            std::vector<glm::mat4> shadowTransforms;
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
            shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

            shadowMapShader_pointLight->bind();
            // 将6个矩阵依次传入着色器数组中
            for (unsigned int i = 0; i < 6; ++i)
                shadowMapShader_pointLight->setUniform4fv("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
            shadowMapShader_pointLight->setUniform1f("far_plane", far_plane);
            shadowMapShader_pointLight->setUniform3f("lightPos", light.position.x, light.position.y, light.position.z);

            glViewport(0, 0, CUBE_SHADOW_SIZE, CUBE_SHADOW_SIZE);
            glBindFramebuffer(GL_FRAMEBUFFER, shadow.fbo);
            glClear(GL_DEPTH_BUFFER_BIT);
            for(auto model : models) model->draw(shadowMapShader_pointLight);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }
    glViewport(0, 0, GlobalSettings::getInstance().GetInt("SCREEN_WIDTH"),GlobalSettings::getInstance().GetInt("SCREEN_HEIGHT"));
}

// 绑定阴影贴图
void Light::bind_shadow(Shader *shader)
{
    shader->bind();
    shader->setUniform4fv("lightProjection", lightProjection);
    shader->setUniform1f("farPlane", far_plane);

    int start_slot_shaderMap = GlobalSettings::getInstance().GetInt("MAX_OBJECT_TEXTURE_SLOTS");

    // 设置纹理数组（后面其实每个单独设置了，但此处提前设置一遍也可）
    // 动态生成 shadowUnits 和 shadowCubeUnits
    std::vector<int> shadowUnits(MAX_SHADOW_MAP_SLOTS);
    std::vector<int> shadowCubeUnits(MAX_SHADOW_MAP_SLOTS);
    for (int i = 0; i < MAX_SHADOW_MAP_SLOTS; ++i) {
        shadowUnits[i] = start_slot_shaderMap + i;
        shadowCubeUnits[i] = start_slot_shaderMap + MAX_SHADOW_MAP_SLOTS + i;
    }
    shader->setUniform1iv("shadowMaps", MAX_SHADOW_MAP_SLOTS, shadowUnits.data());
    shader->setUniform1iv("shadowCubeMaps", MAX_SHADOW_MAP_SLOTS, shadowCubeUnits.data());

    int directionalLightIndex = 0;
    int pointLightIndex = 0;

    for(int i = 0; i<lights.size(); i++){
        int k = start_slot_shaderMap + i;  // 阴影贴图排在普通贴图后面
        if(shadowMaps[i].isDirectional){
            glActiveTexture(GL_TEXTURE0 + k);   
            glBindTexture(GL_TEXTURE_2D, shadowMaps[i].texture);
            // 生成着色器采样器变量名，如 "shadowMap[0]", "shadowMap[1]"
            std::string texUniformName = "shadowMaps[" + std::to_string(directionalLightIndex) + "]"; 
            directionalLightIndex ++;
            shader->setUniform1i(texUniformName, k);
        } else{
            glActiveTexture(GL_TEXTURE0 + k + MAX_SHADOW_MAP_SLOTS);   
            glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMaps[i].texture);
            // 生成着色器采样器变量名，如 "shadowMapCube[0]", "shadowMapCube[1]"
            std::string texUniformName = "shadowCubeMaps[" + std::to_string(pointLightIndex) + "]"; 
            pointLightIndex ++;
            shader->setUniform1i(texUniformName, k + MAX_SHADOW_MAP_SLOTS);
        }
    }
}

// 初始化平行光的阴影贴图（2D纹理）
//
void Light::initDirectionalShadowMap(ShadowMapInfo &shadowInfo)
{
    glGenFramebuffers(1, &shadowInfo.fbo);
    glGenTextures(1, &shadowInfo.texture);
    glBindTexture(GL_TEXTURE_2D, shadowInfo.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GlobalSettings::getInstance().GetInt("SCREEN_WIDTH")* SHADOW_RATE,GlobalSettings::getInstance().GetInt("SCREEN_HEIGHT")* SHADOW_RATE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // 对视野外的颜色做填充
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowInfo.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowInfo.texture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Directional Shadow Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//
// 初始化点光的阴影贴图（立方体纹理）
//
void Light::initPointShadowMap(ShadowMapInfo &shadowInfo)
{
    glGenFramebuffers(1, &shadowInfo.fbo);
    glGenTextures(1, &shadowInfo.texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowInfo.texture);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, CUBE_SHADOW_SIZE, CUBE_SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowInfo.fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowInfo.texture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Point Shadow Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}









glm::vec3 Light::hexToVec3(const std::string& hexStr) {
    std::string hex = hexStr;

    // 移除开头的'#'字符
    if (!hex.empty() && hex[0] == '#') {
        hex.erase(0, 1);
    }

    // 处理三位缩写格式（如"F0F"扩展为"FF00FF"）
    if (hex.length() == 3) {
        std::string expanded;
        for (char c : hex) {
            expanded += std::string(2, c); // 每个字符重复两次
        }
        hex = expanded;
    }

    // 验证长度是否为6位
    if (hex.length() != 6) {
        return glm::vec3(0.0f); // 返回黑色作为默认值
    }

    // 分割RGB分量
    unsigned int r, g, b;
    try {
        r = std::stoul(hex.substr(0, 2), nullptr, 16);
        g = std::stoul(hex.substr(2, 2), nullptr, 16);
        b = std::stoul(hex.substr(4, 2), nullptr, 16);
    } catch (...) {
        return glm::vec3(0.0f); // 解析失败返回黑色
    }

    // 转换为0.0-1.0范围的浮点数
    return glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
}