#pragma once
#include "Renderer.h"
#include <iostream>
#include "Camera.h"
#include "Light.h"
#include "Model.h"
#include "GlobalSettings.h"

void checkGLError(const char* file, int line) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error at " << file << ":" << line << " - " << error << std::endl;
    }
}


void SkyboxPass::execute()
{
    if(ifDeferred){
        Renderer::getInstance().get_DeferedFramebufferForPass()->switch_depth_component_to_default(); // 将深度缓冲切换到默认帧缓冲
    }
    // glDepthMask(GL_FALSE);
    shader->bind();
    texture->bind(shader.get(), TextureType::Cubemap, 0);  // 传入 Shader 指针
    dummyCube->draw();
    // glDepthMask(GL_TRUE);
}

void OpaquePass::execute()
{
    shader->bind();
    shader->setUniform1i("debugMode", Renderer::getInstance().get_debugMode()); // 使用全局调试模式
    // 绑定光照信息
    if(light){
        light->set_sUniform_light(shader.get());
        light->bind_shadow(shader.get()); // 绑定阴影贴图
    }

    for (auto& model : models) {
        model->draw(shader.get());
    }
}

void OpaqueDeferredPass::execute()
{
    // G-Buffer 阶段
    deferred_g_shader->bind();
    deferredFramebuffer->bind(); // 绑定帧缓冲
    glClearBufferfv(GL_COLOR, 0, glm::value_ptr(glm::vec4(0.0f))); // gPosition
    glClearBufferfv(GL_COLOR, 1, glm::value_ptr(glm::vec3(0.0f))); // gNormal
    glClearBufferfv(GL_COLOR, 2, glm::value_ptr(glm::vec4(0.0f))); // gAlbedoSpec
    glClear(GL_DEPTH_BUFFER_BIT);
    for(auto model : models){
        model->draw(deferred_g_shader.get());
    }
    deferredFramebuffer->unbind(); // 解绑帧缓冲

    // SSAO 阶段
    ssaoFrameBuffer->bind(); // 绑定 SSAO 帧缓冲
    glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓存
    ssao_shader->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, deferredFramebuffer->get_texture(0));
    ssao_shader->setUniform1i("gPosition", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, deferredFramebuffer->get_texture(1));
    ssao_shader->setUniform1i("gNormal", 1);
    noiseTexture->bind(ssao_shader.get(), TextureType::Noise, 2); // 绑定噪声纹理
    ssao_shader->setUniform3fv("samples", ssaoKernel.size(), glm::value_ptr(ssaoKernel[0])); // 传入 SSAO 内核
    ssao_shader->setUniform1f("ssaoStrengh", Renderer::getInstance().ssaoStrength); // 设置 SSAO 强度
    dummy_screen->draw();   // 利用屏幕四边形绘制结果
    ssaoFrameBuffer->unbind(); // 解绑 SSAO 帧缓冲

    // 光照阶段
    deferred_l_shader->bind();
    deferred_l_shader->setUniform1i("debugMode", Renderer::getInstance().get_debugMode()); // 使用全局调试模式
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, deferredFramebuffer->get_texture(0));
    deferred_l_shader->setUniform1i("gPosition", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, deferredFramebuffer->get_texture(1));
    deferred_l_shader->setUniform1i("gNormal", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, deferredFramebuffer->get_texture(2));
    deferred_l_shader->setUniform1i("gAlbedoSpec", 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ssaoFrameBuffer->get_texture(0)); // 绑定 SSAO 结果
    deferred_l_shader->setUniform1i("ssao", 3);

    glClear(GL_DEPTH_BUFFER_BIT); // 清除深度缓存
    if(light){
        light->set_sUniform_light(deferred_l_shader.get());
        light->bind_shadow(deferred_l_shader.get()); // 绑定阴影贴图
    }
    dummy_screen->draw();   // 利用屏幕四边形绘制结果
}

void LightPass::execute()
{
    if(ifDeferred){
        Renderer::getInstance().get_DeferedFramebufferForPass()->switch_depth_component_to_default(); // 将深度缓冲切换到默认帧缓冲
    }
    for (auto light : lights) {
        shader->bind();
        light->draw(shader.get());
    }
}

void TransparentPass::execute()
{
    // 计算透明物体的颜色和透明度累积值
    glEnable(GL_BLEND);  // 启用混合
    glBlendFunc(GL_ONE, GL_ONE);  // 计算累积值，所以混合改为简单的相加模式
    oitFramebuffer->bind(); 
    glClear(GL_COLOR_BUFFER_BIT);
    glDepthMask(GL_FALSE);  // 禁用深度测试
    accumShader->bind();
    for(auto model : models){
        model->draw(accumShader.get());
    }
    glDepthMask(GL_TRUE);
    oitFramebuffer->unbind();

    // 接着将上面的累积结果绘制到屏幕四边形当中
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // 改回正常 Alpha 混合
    drawShader->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, oitFramebuffer->get_texture(0));
    drawShader->setUniform1i("accum_texture", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, oitFramebuffer->get_texture(1));
    drawShader->setUniform1i("alpha_texture", 1);
    dummy_screen->draw();   // 利用屏幕四边形绘制结果

    glDisable(GL_BLEND);   // 关闭混合

}

void BakePass::execute()
{
    // 烘焙阴影贴图
    for(auto light : lights){
        light->bakeShadows(shadowMapShader_directionalLight.get(), shadowMapShader_pointLight.get(), models);
    }
}

void ViewPass::execute()
{
    view_texture_shader->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    view_texture_shader->setUniform1i("texture0", 0);
    dummyScreen->draw();
}

void Renderer::initialize()
{
    resizeFBOIfNeeded(GlobalSettings::getInstance().GetInt("SCREEN_WIDTH"), GlobalSettings::getInstance().GetInt("SCREEN_HEIGHT"));
    // 初始化全局资源
    basicShader = std::make_shared<Shader>("res/shader/Basic.shader");
    deferred_g_shader = std::make_shared<Shader>("res/shader/Deferred_G.shader");
    deferred_l_shader = std::make_shared<Shader>("res/shader/Deferred_L.shader");
    pbr_g_shader = std::make_shared<Shader>("res/shader/PBR_G.shader");
    pbr_l_shader = std::make_shared<Shader>("res/shader/PBR_L.shader");

    lightShader = std::make_shared<Shader>("res/shader/Light.shader");

    skyBoxShader = std::make_shared<Shader>("res/shader/Skybox.shader");
    skyBoxCubemap = std::make_shared<Texture>();
    skyBoxCubemap->add_hdri_to_cubemap("res/HDRI.hdr", 1024, true);
    dummyCube = std::make_shared<Mesh>();
    dummyCube->set_mesh_cube();

    transparentAccumShader = std::make_shared<Shader>("res/shader/OIT_accum.shader");
    transparentDrawShader = std::make_shared<Shader>("res/shader/OIT_draw.shader");

    shadowMapShader_directionalLight = std::make_shared<Shader>("res/shader/ShadowMap_directionalLight.shader");
    shadowMapShader_pointLight = std::make_shared<Shader>("res/shader/ShadowMap_pointLight.shader");

    ssao_shader = std::make_shared<Shader>("res/shader/SSAO.shader");

    view_texture_shader = std::make_shared<Shader>("res/shader/View_texture.shader");

    dummyScreen = std::make_shared<Mesh>();
    dummyScreen->set_mesh_screen();

    // TEST
    brdfLUT = std::make_unique<Texture>();
    brdfLUT->add_preCal_CT_BRDF(512); // 添加brdfLUT纹理
}

void Renderer::setupRenderPasses()
{
    renderPasses.clear();
    // 注册渲染通道
    // renderPasses.push_back(std::make_unique<BakePass>(shadowMapShader_directionalLight,shadowMapShader_pointLight, lights, renderType_model_map[RenderType::Basic]));
    // renderPasses.push_back(std::make_unique<OpaquePass>(basicShader, renderType_model_map[RenderType::Basic], renderType_light_map[RenderType::Basic]));
    // renderPasses.push_back(std::make_unique<OpaqueDeferredPass>(deferred_g_shader, deferred_l_shader, ssao_shader, deferredFramebuffer, ssaoFrameBuffer, renderType_model_map[RenderType::Basic], renderType_light_map[RenderType::Basic], dummyScreen));
    renderPasses.push_back(std::make_unique<PBRPass>(pbr_g_shader, pbr_l_shader, ssao_shader, pbrDeferredFramebuffer, ssaoFrameBuffer, skyBoxCubemap, renderType_model_map[RenderType::Basic], dummyScreen));
    renderPasses.push_back(std::make_unique<SkyboxPass>(skyBoxShader, skyBoxCubemap, dummyCube, true));
    // renderPasses.push_back(std::make_unique<LightPass>(lightShader, lights, true));
    renderPasses.push_back(std::make_unique<ImGuiPass>()); // 添加 ImGui 渲染通道


    //renderPasses.push_back(std::make_unique<TransparentPass>(transparentAccumShader, transparentDrawShader, oitFramebuffer, renderType_model_map[RenderType::Transparent], dummyScreen));

    // renderPasses.push_back(std::make_unique<ViewPass>(view_texture_shader, brdfLUT->get_textureID(0), dummyScreen));
}

void Renderer::resizeFBOIfNeeded(int screenWidth, int screenHeight)
{
    if (screenWidth != currentWidth || screenHeight != currentHeight) {
        std::vector<Framebuffer::AttachmentConfig> attachments = {
            {GL_RGBA16F, GL_RGBA, GL_FLOAT},
            {GL_RGB16F, GL_RGB, GL_FLOAT},
            {GL_RGBA16F, GL_RGBA, GL_FLOAT}
        };
        deferredFramebuffer = std::make_shared<Framebuffer>(screenWidth, screenHeight, attachments, true, false);

        attachments = {
            {GL_RGBA16F, GL_RGBA, GL_FLOAT},
            {GL_RGB16F, GL_RGB, GL_FLOAT},
            {GL_RGB16F, GL_RGB, GL_FLOAT},
            {GL_RGB16F, GL_RGB, GL_FLOAT}
        };
        pbrDeferredFramebuffer = std::make_shared<Framebuffer>(screenWidth, screenHeight, attachments, true, false);

        attachments = {
            {GL_RGBA16F, GL_RGBA, GL_FLOAT},
            {GL_R16F, GL_RED, GL_FLOAT}
        };
        oitFramebuffer = std::make_shared<Framebuffer>(screenWidth, screenHeight, attachments, false, false);

        attachments = {
            {GL_RED, GL_RGB, GL_FLOAT}
        };
        ssaoFrameBuffer = std::make_shared<Framebuffer>(screenWidth, screenHeight, attachments, false, false);

        // 更新当前宽高
        currentWidth = screenWidth;
        currentHeight = screenHeight;
    }
}

void ImGuiPass::execute()
{
    // 开始新一帧 ImGui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug Window");
    ImGui::SliderFloat("SSAO strengh", &Renderer::getInstance().ssaoStrength, 0.0f, 5.0f);
    ImGui::End();

    // 渲染 ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void PBRPass::execute()
{
    // G-Buffer 阶段
    pbr_g_shader->bind();
    pbrDeferredFramebuffer->bind(); // 绑定帧缓冲
    glClearBufferfv(GL_COLOR, 0, glm::value_ptr(glm::vec4(0.0f))); // gPosition
    glClearBufferfv(GL_COLOR, 1, glm::value_ptr(glm::vec3(0.0f))); // gNormal
    glClearBufferfv(GL_COLOR, 2, glm::value_ptr(glm::vec3(0.0f))); // gAlbedo
    glClearBufferfv(GL_COLOR, 3, glm::value_ptr(glm::vec3(0.0f))); // gMetallicRoughnessAO
    glClear(GL_DEPTH_BUFFER_BIT);
    for(auto model : models){
        model->draw(pbr_g_shader.get());
    }
    pbrDeferredFramebuffer->unbind(); // 解绑帧缓冲

    // SSAO 阶段
    ssaoFrameBuffer->bind(); // 绑定 SSAO 帧缓冲
    glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓存
    ssao_shader->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pbrDeferredFramebuffer->get_texture(0));
    ssao_shader->setUniform1i("gPosition", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pbrDeferredFramebuffer->get_texture(1));
    ssao_shader->setUniform1i("gNormal", 1);
    noiseTexture->bind(ssao_shader.get(), TextureType::Noise, 2); // 绑定噪声纹理
    ssao_shader->setUniform3fv("samples", ssaoKernel.size(), glm::value_ptr(ssaoKernel[0])); // 传入 SSAO 内核
    ssao_shader->setUniform1f("ssaoStrengh", Renderer::getInstance().ssaoStrength); // 设置 SSAO 强度
    dummy_screen->draw();   // 利用屏幕四边形绘制结果
    ssaoFrameBuffer->unbind(); // 解绑 SSAO 帧缓冲

    // 光照阶段
    pbr_l_shader->bind();
    // pbr_l_shader->setUniform1i("debugMode", Renderer::getInstance().get_debugMode()); // 使用全局调试模式
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pbrDeferredFramebuffer->get_texture(0));
    pbr_l_shader->setUniform1i("gPosition", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pbrDeferredFramebuffer->get_texture(1));
    pbr_l_shader->setUniform1i("gNormal", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, pbrDeferredFramebuffer->get_texture(2));
    pbr_l_shader->setUniform1i("gAlbedo", 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, pbrDeferredFramebuffer->get_texture(3));
    pbr_l_shader->setUniform1i("gMetallicRoughnessAO", 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, ssaoFrameBuffer->get_texture(0)); // 绑定 SSAO 结果
    pbr_l_shader->setUniform1i("ssao", 4);
    brdfLUT->bind(pbr_l_shader.get(), TextureType::BRDF, 5);   // 绑定 brdfLUT
    prefilterMap->bind(pbr_l_shader.get(), TextureType::Prefilter, 6); // 绑定预过滤的立方体贴图

    glClear(GL_DEPTH_BUFFER_BIT); // 清除深度缓存
    dummy_screen->draw();   // 利用屏幕四边形绘制结果
}

std::vector<glm::vec3> generateSSAOKernel(int kernelSize)
{
    std::vector<glm::vec3> kernel;
    kernel.reserve(kernelSize);

    for (int i = 0; i < kernelSize; ++i) {
        glm::vec3 sample(
            (float)rand() / RAND_MAX * 2.0f - 1.0f, // x in [-1, 1]
            (float)rand() / RAND_MAX * 2.0f - 1.0f, // y in [-1, 1]
            (float)rand() / RAND_MAX                // z in [0, 1]
        );
        sample = glm::normalize(sample);
        sample *= (float)rand() / RAND_MAX;

        // 让靠近原点的点分布更密集（指数分布）
        float scale = (float)i / kernelSize;
        scale = glm::mix(0.1f, 1.0f, scale * scale);
        sample *= scale;

        kernel.push_back(sample);
    }

    return kernel;
}