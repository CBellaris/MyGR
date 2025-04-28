#pragma once
#include <glad/glad.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>

#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

class Model;
class Camera;
class Light;
class Renderer;

#define GL_CALL(func) \
    func; \
    checkGLError(__FILE__, __LINE__)

void checkGLError(const char* file, int line);

std::vector<glm::vec3> generateSSAOKernel(int kernelSize);   

enum class RenderType{
    Basic,
    Transparent
};
// 按绘制方法，拆分渲染流程
class RenderPass {
    public:
        virtual ~RenderPass() = default;
        virtual void execute() = 0;  // 纯虚函数，具体实现由子类完成
    };

// 天空盒绘制
class SkyboxPass : public RenderPass {
public:
    SkyboxPass(std::shared_ptr<Shader> shader, std::shared_ptr<Texture> texture, std::shared_ptr<Mesh> dummyCube, bool ifDeferred = false)
        : shader(shader), texture(texture), dummyCube(dummyCube), ifDeferred(ifDeferred) {}

    void execute() override;

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<Mesh> dummyCube;

    bool ifDeferred; // 是否使用延迟渲染
};

// 不透明物体绘制
class OpaquePass : public RenderPass {
public:
    OpaquePass(std::shared_ptr<Shader> basicShader,
        std::vector<std::shared_ptr<Model>>& models,
        std::shared_ptr<Light> light)
            : shader(basicShader), models(models), light(light) {}
    
    void execute() override;

private:
    std::shared_ptr<Shader> shader;
    std::vector<std::shared_ptr<Model>>& models;
    std::shared_ptr<Light> light;
    
};

// 不透明物体绘制，延迟渲染
class OpaqueDeferredPass : public RenderPass {
    public:
    OpaqueDeferredPass(std::shared_ptr<Shader> deferred_g_shader,
            std::shared_ptr<Shader> deferred_l_shader,
            std::shared_ptr<Shader> ssao_shader,
            std::shared_ptr<Framebuffer> deferredFramebuffer,
            std::shared_ptr<Framebuffer> ssaoFrameBuffer,
            std::vector<std::shared_ptr<Model>>& models,
            std::shared_ptr<Light> light,
            std::shared_ptr<Mesh> dummy_screen)
                : deferred_g_shader(deferred_g_shader), deferred_l_shader(deferred_l_shader), ssao_shader(ssao_shader), 
                deferredFramebuffer(deferredFramebuffer), ssaoFrameBuffer(ssaoFrameBuffer),
                models(models), light(light), dummy_screen(dummy_screen){
                    noiseTexture = std::make_unique<Texture>();
                    noiseTexture->add_noise_texture(); // 添加噪声纹理

                    ssaoKernel = generateSSAOKernel(64); // 生成SSAO采样内核
                }
        
        void execute() override;

    private:
        std::shared_ptr<Shader> deferred_g_shader;
        std::shared_ptr<Shader> deferred_l_shader;
        std::shared_ptr<Shader> ssao_shader;
        std::vector<std::shared_ptr<Model>>& models;
        std::shared_ptr<Light> light;

        std::unique_ptr<Texture> noiseTexture; // 噪声纹理
        std::vector<glm::vec3> ssaoKernel; // SSAO采样内核
        std::shared_ptr<Mesh> dummy_screen;

        std::shared_ptr<Framebuffer> deferredFramebuffer;
        std::shared_ptr<Framebuffer> ssaoFrameBuffer;
};

// PBR管线绘制
class PBRPass : public RenderPass {
public:
    PBRPass(std::shared_ptr<Shader> pbr_g_shader,
            std::shared_ptr<Shader> pbr_l_shader,
            std::shared_ptr<Shader> ssao_shader,
            std::shared_ptr<Framebuffer> pbrDeferredFramebuffer,
            std::shared_ptr<Framebuffer> ssaoFrameBuffer,
            std::shared_ptr<Texture> prefilterMap,
            std::vector<std::shared_ptr<Model>>& models,
            std::shared_ptr<Mesh> dummy_screen)
                : pbr_g_shader(pbr_g_shader), pbr_l_shader(pbr_l_shader), ssao_shader(ssao_shader), 
                pbrDeferredFramebuffer(pbrDeferredFramebuffer), ssaoFrameBuffer(ssaoFrameBuffer),
                prefilterMap(prefilterMap),
                models(models), dummy_screen(dummy_screen){
                    noiseTexture = std::make_unique<Texture>();
                    noiseTexture->add_noise_texture(); // 添加噪声纹理
                    ssaoKernel = generateSSAOKernel(64); // 生成SSAO采样内核

                    brdfLUT = std::make_unique<Texture>();
                    brdfLUT->add_preCal_CT_BRDF(512); // 添加brdfLUT纹理

                    
                }
        
        void execute() override;

private:
    std::shared_ptr<Shader> pbr_g_shader;
    std::shared_ptr<Shader> pbr_l_shader;
    std::shared_ptr<Shader> ssao_shader;
    std::vector<std::shared_ptr<Model>>& models;

    std::unique_ptr<Texture> noiseTexture; // 噪声纹理
    std::unique_ptr<Texture> brdfLUT; // brdfLUT
    std::shared_ptr<Texture> prefilterMap; // 预过滤的立方体贴图

    std::vector<glm::vec3> ssaoKernel; // SSAO采样内核
    std::shared_ptr<Mesh> dummy_screen;

    std::shared_ptr<Framebuffer> pbrDeferredFramebuffer;
    std::shared_ptr<Framebuffer> ssaoFrameBuffer;
};

// 透明物体绘制
class TransparentPass : public RenderPass {
public:
    TransparentPass(std::shared_ptr<Shader> accumShader,
        std::shared_ptr<Shader> drawShader,
        std::shared_ptr<Framebuffer> oitFramebuffer,
        std::vector<std::shared_ptr<Model>>& models,
        std::shared_ptr<Mesh> dummy_screen)
        : accumShader(accumShader), drawShader(drawShader), oitFramebuffer(oitFramebuffer),
        models(models), dummy_screen(dummy_screen) {}

    void execute() override;

private:
    std::shared_ptr<Shader> accumShader;
    std::shared_ptr<Shader> drawShader;
    std::vector<std::shared_ptr<Model>>& models;
    std::shared_ptr<Mesh> dummy_screen;

    std::shared_ptr<Framebuffer> oitFramebuffer;
};
        
// 灯光绘制
class LightPass : public RenderPass {
public:
    LightPass(std::shared_ptr<Shader> lightShader, std::vector<std::shared_ptr<Light>>& lights, bool ifDeferred = false)
        : shader(lightShader), lights(lights), ifDeferred(ifDeferred) {}

    void execute() override;

private:
    bool ifDeferred; // 是否使用延迟渲染
    std::shared_ptr<Shader> shader;
    std::vector<std::shared_ptr<Light>>& lights;
};
       
// 烘焙
class BakePass : public RenderPass {
public:
    BakePass(std::shared_ptr<Shader> shadowMapShader_directionalLight, 
        std::shared_ptr<Shader> shadowMapShader_pointLight, 
        std::vector<std::shared_ptr<Light>>& lights, 
        std::vector<std::shared_ptr<Model>>& models)
        : shadowMapShader_directionalLight(shadowMapShader_directionalLight), shadowMapShader_pointLight(shadowMapShader_pointLight), 
        lights(lights), models(models) {}

    void execute() override;

private:
    std::shared_ptr<Shader> shadowMapShader_directionalLight;
    std::shared_ptr<Shader> shadowMapShader_pointLight;
    std::vector<std::shared_ptr<Light>>& lights;
    std::vector<std::shared_ptr<Model>>& models;
};

// ImGui 绘制
class ImGuiPass : public RenderPass {
public:
    ImGuiPass(){}    
    void execute() override;
    
private:
    // ImGui 相关成员变量和方法
    // 例如：ImGui上下文、窗口、UI元素等
};


// 预览FBO
class ViewPass : public RenderPass {
public:
    ViewPass(std::shared_ptr<Shader> view_texture_shader, 
        GLuint textureID,
        std::shared_ptr<Mesh> dummyScreen)
        : view_texture_shader(view_texture_shader), textureID(textureID), dummyScreen(dummyScreen){}

    void execute() override;

private:
    std::shared_ptr<Shader> view_texture_shader;
    GLuint textureID;
    std::shared_ptr<Mesh> dummyScreen;
};

class Renderer {
public:
    static Renderer& getInstance() {
        static Renderer instance;
        return instance;
    }

    // 初始化
    void initialize();
    void setupRenderPasses();
    void resizeFBOIfNeeded(int screenWidth, int screenHeight);

    // 调用绘制
    void renderAll() {
        for (auto& pass : renderPasses) {
            pass->execute();
        }
    }

    // 注册场景对象(灯光和模型等)
    void add_light(std::shared_ptr<Light> light) {
        if (light) {
            lights.push_back(light);
        }
    }
    void set_model_renderType(std::shared_ptr<Model> model, RenderType rt = RenderType::Basic) {
        // 从旧 Shader 组移除 Model
        for (auto& [s, modelList] : renderType_model_map) {
            auto it = std::find(modelList.begin(), modelList.end(), model);
            if (it != modelList.end()) {
                modelList.erase(it);
                break;
            }
        }

        // 添加 Model 到新 Shader 组
        renderType_model_map[rt].push_back(model);
    }
    void set_light_renderType(RenderType rt, std::shared_ptr<Light> light) {
        renderType_light_map[rt] = light;
    }

    // 设置调试模式
    void set_debugMode(int mode) {
        debugMode = mode;
    }
    // 获取当前的调试模式
    int get_debugMode() const {
        return debugMode;
    }
    std::shared_ptr<Framebuffer> get_DeferedFramebufferForPass() const {
        for (auto& pass : renderPasses){
            if (dynamic_cast<OpaqueDeferredPass*>(pass.get())) {
                return deferredFramebuffer; // 如果是 OpaqueDeferredPass，返回 deferredFramebuffer
            } else if (dynamic_cast<PBRPass*>(pass.get())) {
                return pbrDeferredFramebuffer; // 如果是 PBRPass，返回 pbrDeferredFramebuffer
            }
        }
        return nullptr; // 如果不是这两种类型，返回空指针
    }

public:
    float ssaoStrength = 1.0f; // SSAO强度

private:
    Renderer() = default; // 私有构造函数，禁止外部实例化
    ~Renderer() = default;

    std::vector<std::unique_ptr<RenderPass>> renderPasses; // 渲染通道列表
    std::vector<std::unique_ptr<RenderPass>> renderPasses_deferred; // 延迟渲染通道列表

    // Framebuffer对象
    std::shared_ptr<Framebuffer> deferredFramebuffer;   // 延迟渲染-几何阶段
    std::shared_ptr<Framebuffer> oitFramebuffer;        // OIT-累积阶段
    std::shared_ptr<Framebuffer> ssaoFrameBuffer;       // SSAO-颜色缓冲区
    std::shared_ptr<Framebuffer> pbrDeferredFramebuffer; // PBR-几何阶段

    // 着色器资源
    std::shared_ptr<Shader> basicShader;    // 基础着色
    std::shared_ptr<Shader> lightShader;    // 标示灯光
    std::shared_ptr<Shader> skyBoxShader;   // 天空盒
    std::shared_ptr<Shader> transparentAccumShader; // OIT累积
    std::shared_ptr<Shader> transparentDrawShader;  // OIT绘制
    std::shared_ptr<Shader> shadowMapShader_directionalLight;   // 阴影贴图-平行光
    std::shared_ptr<Shader> shadowMapShader_pointLight;         // 阴影贴图-点光
    std::shared_ptr<Shader> view_texture_shader;    // 预览FBO
    std::shared_ptr<Shader> deferred_g_shader;      // 延迟渲染-几何阶段
    std::shared_ptr<Shader> deferred_l_shader;      // 延迟渲染-光照阶段
    std::shared_ptr<Shader> ssao_shader;           // SSAO着色器
    std::shared_ptr<Shader> pbr_g_shader;          // PBR-几何阶段
    std::shared_ptr<Shader> pbr_l_shader;          // PBR-光照阶段

    std::shared_ptr<Texture> skyBoxCubemap;
    std::shared_ptr<Mesh> dummyCube;

    // 场景对象
    std::vector<std::shared_ptr<Light>> lights;
    std::unordered_map<RenderType, std::vector<std::shared_ptr<Model>>> renderType_model_map;
    std::unordered_map<RenderType, std::shared_ptr<Light>> renderType_light_map;

    std::shared_ptr<Mesh> dummyScreen;

    // TEST
    std::shared_ptr<Texture> brdfLUT; 


    int debugMode = 0; // 调试模式，默认值为 0
    int currentWidth;
    int currentHeight;
};



