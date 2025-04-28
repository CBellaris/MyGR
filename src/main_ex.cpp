#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <memory>

#include "BufferObject.h"
#include "Shader.h"
#include "Camera.h"
#include "InputManager.h"
#include "Renderer.h"
#include "Light.h"
#include "Model.h"
#include "GlobalSettings.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                               GLsizei length, const GLchar* message, const void* userParam);


int main(void)
{
    // 加载设置
    if (!GlobalSettings::getInstance().LoadFromFile("res/settings.json")) {
        std::cerr << "Failed to load settings.\n";
    }

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    // 设置一些参数
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);    // MSAA
    

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(GlobalSettings::getInstance().GetInt("SCREEN_WIDTH"), GlobalSettings::getInstance().GetInt("SCREEN_HEIGHT"), "MyGL", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // 让我们的鼠标不会移动到窗口外

    // 加载glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 创建 ImGui 上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 启用键盘控制（可选）

    // 设置风格
    ImGui::StyleColorsDark();

    // 初始化 ImGui GLFW 和 OpenGL3 实现
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    
    // 启用调试输出
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);  // 确保调试信息立即输出
    // 注册调试回调函数
    glDebugMessageCallback(debugCallback, nullptr);
    // 定义视口大小
    glViewport(0, 0, GlobalSettings::getInstance().GetInt("SCREEN_WIDTH"), GlobalSettings::getInstance().GetInt("SCREEN_HEIGHT"));
    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    // 启用背面剔除
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // MSAA
    glEnable(GL_MULTISAMPLE);

    // 注册窗口大小变更回调函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    // 创建摄像机
    Camera* camera = new Camera();
    camera->setCameraLookAt(glm::vec3(0.0f));

    // // 创建灯光
    // std::shared_ptr<Light> light = std::make_shared<Light>();
    // // 点光1
    // LightUnit lightUnit_1;
    // lightUnit_1.intensity = glm::vec3(0.4f, 0.8f, 0.8f);
    // lightUnit_1.position = glm::vec3(0.0f, 3.5f, 0.0f);
    // light->add_light(lightUnit_1);
    // // 点光2
    // LightUnit lightUnit_2;
    // lightUnit_2.intensity = glm::vec3(0.3f, 0.7f, 0.7f);
    // lightUnit_2.position = glm::vec3(3.0f, 2.8f, 3.0f);
    // lightUnit_2.color = Light::hexToVec3("#29a0ca");
    // light->add_light(lightUnit_2);
    // // 点光3
    // LightUnit lightUnit_3;
    // lightUnit_3.intensity = glm::vec3(0.3f, 0.7f, 0.7f);
    // lightUnit_3.position = glm::vec3(0.0f, 3.5f, 3.5f);
    // light->add_light(lightUnit_3);
    // // 点光4
    // LightUnit lightUnit_4;
    // lightUnit_4.intensity = glm::vec3(0.3f, 0.7f, 0.7f);
    // lightUnit_4.position = glm::vec3(0.0f, 3.5f, -3.5f);
    // light->add_light(lightUnit_4);
    //平行光
    // LightUnit lightUnit_3;
    // lightUnit_3.isDirectional = 1;
    // lightUnit_3.intensity = glm::vec3(0.1f, 0.4f, 0.4f);
    // lightUnit_3.direction = glm::vec3(0.0f, -1.0f, -1.0f);
    // light->add_light(lightUnit_3);
    // Renderer::getInstance().add_light(light);
    // Renderer::getInstance().set_light_renderType(RenderType::Basic, light);

    
    // // 创建立方体
    // Texture* tex_cube = new Texture();
    // tex_cube->add_image("res/diffuse_brick_2.jpg", TextureType::Diffuse);
    // tex_cube->add_image("res/normal_brick_2.jpg", TextureType::Normal);
    // tex_cube->add_image("res/height_brick_2.jpg", TextureType::Height);
    // std::shared_ptr<Model> cube = std::make_shared<Model>();
    // cube->add_basic_geom(BasicGeom::Cube, tex_cube);
    // Renderer::getInstance().set_model_renderType(cube, RenderType::Basic);

    // // 创建地板
    // Texture* tex_floor = new Texture();
    // tex_floor->add_image("res/diffuse_floor.jpg", TextureType::Diffuse);
    // std::shared_ptr<Model> floor = std::make_shared<Model>();
    // floor->add_basic_geom(BasicGeom::Plane, tex_floor);
    // floor->set_scale(glm::vec3(10.0f, 10.0f, 10.0f));
    // floor->set_position(glm::vec3(0.0f, -1.5f, 0.0f));
    // Renderer::getInstance().set_model_renderType(floor, RenderType::Basic);



    // // 添加透明物体
    // // 加载窗户纹理
    // Texture* win_tex = new Texture();
    // win_tex->add_image("res/blending_transparent_window.png", TextureType::Diffuse);
    // // 窗户1
    // std::shared_ptr<Model> window1 = std::make_shared<Model>();
    // window1->add_basic_geom(BasicGeom::Plane, win_tex);
    // window1->set_position(glm::vec3(0.0f, 0.0f, 1.3f));
    // // 窗户2
    // std::shared_ptr<Model> window2 = std::make_shared<Model>();
    // window2->add_basic_geom(BasicGeom::Plane, win_tex);
    // window2->set_position(glm::vec3(0.0f, 0.0f, 2.0f));
    // // 注册窗户的渲染方式
    // Renderer::getInstance().set_model_renderType(window1, RenderType::Transparent);
    // Renderer::getInstance().set_model_renderType(window2, RenderType::Transparent);

    // PBR测试
    Texture* tex_sphere = new Texture();
    tex_sphere->add_image("res/scratchMetal_diffuse.jpg", TextureType::Diffuse);
    tex_sphere->add_image("res/scratchMetal_normal.jpg", TextureType::Normal);
    tex_sphere->add_image("res/scratchMetal_metallic.jpg", TextureType::Metallic);
    tex_sphere->add_image("res/scratchMetal_roughness.jpg", TextureType::Roughness);
    tex_sphere->add_image("res/scratchMetal_ao.jpg", TextureType::AO);
    std::shared_ptr<Model> sphere = std::make_shared<Model>();
    sphere->add_basic_geom(BasicGeom::Sphere, tex_sphere);
    Renderer::getInstance().set_model_renderType(sphere, RenderType::Basic);

    float deltaTime = 0.0f; // 当前帧与上一帧的时间差
    float lastTime = 0.0f; // 上一帧的时间
    float currentTime = 0.0f;

    // 初始化渲染流程
    Renderer::getInstance().initialize();
    Renderer::getInstance().setupRenderPasses();

    // 处理键盘鼠标输入
    InputManager& inputManager = InputManager::getInstance(); // 单例类
    inputManager.setCamera(camera);
    inputManager.setDeltaTime(&deltaTime);
    glfwSetKeyCallback(window, InputManager::keyCallback);
    glfwSetCursorPosCallback(window, InputManager::mouse_pos_callback);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色缓存，深度缓存

        // 渲染场景
        Renderer::getInstance().renderAll();

        // 计算帧时间
        currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        inputManager.update(window); // 更新输入
        
        // 检查并调用事件，交换缓冲
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    delete camera;

    // 释放ImGUI资源
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // 保存设置
    GlobalSettings::getInstance().SaveToFile("res/settings.json");

    glfwTerminate();
    return 0;
}


// 回调函数，当窗口大小改变时，GLFW会调用这个函数，我们在这里同步改变OpenGL的视口大小
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// 用于错误处理的回调函数
void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                               GLsizei length, const GLchar* message, const void* userParam) {
    // 获取错误的源和类型信息
    std::string sourceStr;
    switch (source) {
        case GL_DEBUG_SOURCE_API: sourceStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: sourceStr = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION: sourceStr = "Application"; break;
        case GL_DEBUG_SOURCE_OTHER: sourceStr = "Other"; break;
    }

    std::string typeStr;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR: typeStr = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "Undefined Behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY: typeStr = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "Performance"; break;
        case GL_DEBUG_TYPE_OTHER: typeStr = "Other"; break;
    }

    std::string severityStr;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: severityStr = "High"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "Medium"; break;
        case GL_DEBUG_SEVERITY_LOW: severityStr = "Low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
    }

    // 打印出错误的详细信息
    std::cout << "OpenGL Debug Message: " << std::endl;
    std::cout << "  Source: " << sourceStr << std::endl;
    std::cout << "  Type: " << typeStr << std::endl;
    std::cout << "  Severity: " << severityStr << std::endl;
    std::cout << "  Message: " << message << std::endl;
}