#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include <vector>

#include "BufferObject.h"

class Shader;

class Camera
{
private:
    // 摄像机外属性
    glm::vec3 pos;
    glm::vec3 target;
    glm::vec3 direction;
    glm::vec3 up;
    glm::vec3 cameraRight;
    glm::vec3 cameraUp;
    bool lockTarget; 
    glm::mat4 viewMatrix; 
    
    // 摄像机内属性
    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;
    bool orthographic;

    // 投影矩阵
    glm::mat4 projectionMatrix;

    // 最终的摄像机变换矩阵
    glm::mat4 viewProjectionMatrix;

    // 摄像机移动速度和鼠标灵敏度
    float cameraSpeed;
    float cameraSensitivity;

    // UBO
    UBO* cameraUBO;

public:
    Camera();
    ~Camera();

    // 设置属性
    void setCameraPosition(const glm::vec3& pos);
    void setCameraDirection(const glm::vec3& direction);
    void setCameraLookAt(const std::optional<glm::vec3>& newTarget);

    void setPerspective(float fov, float aspectRatio, float nearPlane, float farPlane);
    void setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    void setProjectionMode(bool useOrthographic);

    // 一些访问接口
    inline const glm::mat4& getViewMatrix() const {return viewMatrix;}
    inline const glm::mat4& getViewProjectionMatrix() const {return viewProjectionMatrix;}

    inline const float getCameraSensitivity() const {return cameraSensitivity;}
    inline const glm::vec3 getCameradirection() const {return direction;}

    // 处理按键
    void processKey(bool Press_W, bool Press_A, bool Press_S, bool Press_D, float deltaTime);
    
private:
    void updateViewMatrix();
    void updateCameraVectors();
    void updateProjectionMatrix();
    void updataViewProjectionMatrix();
};