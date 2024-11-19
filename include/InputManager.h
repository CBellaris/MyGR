#include <GLFW/glfw3.h>
#include "Camera.h"
#include <cmath>

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

        if (camera) {
            camera->processKey(Press_W, Press_A, Press_S, Press_D, *deltaTime);
        }

        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    }

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) 
    {
        InputManager& instance = getInstance();  // 获取单例实例

        if (instance.camera) 
        {
        }
    }

    static void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos)
    {
        InputManager& instance = getInstance();  // 获取单例实例

        if(instance.firstMouse)
        {
            instance.lastX = xpos;
            instance.lastY = ypos;
            instance.pitch = glm::degrees(asin(instance.camera->getCameradirection().y));
            instance.yaw = glm::degrees(atan2(instance.camera->getCameradirection().z, instance.camera->getCameradirection().x));
            instance.firstMouse = false;
        }
        else
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
};