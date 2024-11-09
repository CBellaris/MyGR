#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>

#include "BufferObject.h"
#include "Shader.h"
#include "Texture.h"

static void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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


int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    // 设置一些参数
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(800, 600, "Hello World", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // 加载glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 启用调试输出
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);  // 确保调试信息立即输出

    // 注册调试回调函数
    glDebugMessageCallback(debugCallback, nullptr);

    // 定义视口大小
    glViewport(0, 0, 800, 600);

    // 注册窗口大小变更回调函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 定义顶点数据
    std::vector<float> vertices = {
        -0.5f,-0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 1.0f, 0.0f,
        0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 1.0f
    };

    // 定义索引数组
    std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 3, 0
    };


    // 使用自定义的类，管理一个VAO，并绑定VBO和IBO
    VertexArrayObject* VAO = new VertexArrayObject();
    VAO->addVertexBuffer(vertices);
    VAO->addIndexBuffer(indices);
    VAO->push<float>(2);
    VAO->push<float>(2);
    VAO->bindAll();

    // 读取shader源码
    Shader* shader = new Shader("res/shader/Basic.shader");
    shader->setUniform4f("color_b", 0.1, 0.5, 0.7, 1.0);

    // 创建纹理
    Texture texture = Texture();
    texture.add_image("res/pic1.jpg");
    texture.bind(shader);

    //绘制前绑定VAO和shader
    shader->bind();
    VAO->bind();

    float color_b = 0.7f;
    float rate = 0.01f;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader->setUniform4f("color_b", 0.1, 0.5, color_b, 1.0);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        processInput(window);
        // 检查并调用事件，交换缓冲
        glfwPollEvents();
        glfwSwapBuffers(window);

        color_b += rate;
        if (color_b >= 1.0f)
        {
            color_b = 0.0f;
        }
        
    }

    delete VAO;
    delete shader;
    glfwTerminate();
    return 0;
}