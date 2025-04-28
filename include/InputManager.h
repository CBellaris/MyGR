#include <GLFW/glfw3.h>
#include "Camera.h"
#include <cmath>
#include "Renderer.h"

#ifndef M_PI
#define M_PI 3.1416
#endif

class InputManager {
public:
    static InputManager& getInstance() {
        static InputManager instance; // 唯一的实例，局部静态变量，第一次调用时创建，线程安全
        return instance;
    }

    // 删除拷贝构造函数和赋值运算符，防止其他人复制或赋值单例实例
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    // 设置 camera 和 deltaTime
    void setCamera(Camera* cam) {
        camera = cam;
    }

    void setDeltaTime(float* dt) {
        deltaTime = dt;
    }

    void update(GLFWwindow* window) 
    {
        bool Press_W = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        bool Press_A = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
        bool Press_S = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        bool Press_D = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

        bool Press_0 = glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS;
        bool Press_1 = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
        bool Press_2 = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
        bool Press_3 = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;
        bool Press_4 = glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS;
        bool Press_5 = glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS;

        if (Press_0) debugMode = 0;
        if (Press_1) debugMode = 1;
        if (Press_2) debugMode = 2;
        if (Press_3) debugMode = 3;
        if (Press_4) debugMode = 4;
        if (Press_5) debugMode = 5;

        Renderer::getInstance().set_debugMode(debugMode); // 设置调试模式

        if (camera) {
            camera->processKey(Press_W, Press_A, Press_S, Press_D, *deltaTime);
        }

        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    }

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) 
    {
        InputManager& instance = getInstance();  // 获取单例实例

        if(!instance.mouseLocked){
            ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods); // 转发给 ImGui
        }

        if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
            if(instance.mouseLocked) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // 释放鼠标
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // 锁定鼠标
            }
            instance.mouseLocked = !instance.mouseLocked; // 切换状态
        }

    }

    static void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos)
    {
        ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos); // 转发给 ImGui

        InputManager& instance = getInstance();  // 获取单例实例

        if(instance.firstMouse)
        {
            instance.lastX = xpos;
            instance.lastY = ypos;
            instance.pitch = glm::degrees(asin(instance.camera->getCameradirection().y));
            instance.yaw = glm::degrees(atan2(instance.camera->getCameradirection().z, instance.camera->getCameradirection().x));
            instance.firstMouse = false;
        }
        else if(instance.mouseLocked) // 只有在鼠标锁定时才处理鼠标移动事件
        {
            float xoffset = xpos - instance.lastX;
            float yoffset = instance.lastY - ypos; 
            instance.lastX = xpos;
            instance.lastY = ypos;

            xoffset *= instance.camera->getCameraSensitivity();
            yoffset *= instance.camera->getCameraSensitivity();

            instance.yaw   += xoffset;
            instance.pitch += yoffset;

            if(instance.pitch > 89.0f)
                instance.pitch = 89.0f;
            if(instance.pitch < -89.0f)
                instance.pitch = -89.0f;

            glm::vec3 front;
            front.x = cos(glm::radians(instance.yaw)) * cos(glm::radians(instance.pitch));
            front.y = sin(glm::radians(instance.pitch));
            front.z = sin(glm::radians(instance.yaw)) * cos(glm::radians(instance.pitch));
            instance.camera->setCameraDirection(front);
        }
    }

private:
    InputManager() = default; // 构造函数设为私有，确保不能外部实例化
    Camera* camera = nullptr;
    float* deltaTime = nullptr;
    bool firstMouse = true;
    float lastX = 0.0f;
    float lastY = 0.0f;
    float yaw = 0.0f;
    float pitch = 0.0f;

    bool mouseLocked = true;

    int debugMode = 0;
};