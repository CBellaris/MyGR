#include "Camera.h"
#include "Shader.h"
#include "GlobalSettings.h"

Camera::Camera() : 
    pos(glm::vec3(0.0f, 0.0f, 1.0f)), 
    target(glm::vec3(0.0f)), 
    up(glm::vec3(0.0f, 1.0f, 0.0f)), 
    lockTarget(false),
    fov(45.0f),
    aspectRatio(16.0f / 9.0f),
    nearPlane(0.1f),
    farPlane(50.0f),
    orthographic(false),
    cameraSpeed(0.8f),
    cameraSensitivity(0.3f)
{
    aspectRatio = (float)GlobalSettings::getInstance().GetInt("SCREEN_WIDTH") / (float)GlobalSettings::getInstance().GetInt("SCREEN_HEIGHT");

    cameraUBO = new UBO (sizeof(glm::mat4)+sizeof(glm::vec3), 0); // 创建 UBO（存储一个 mat4和一个vec3）, 绑定点 0
    cameraUBO->PushData(viewProjectionMatrix);
    cameraUBO->PushData(pos);

    direction = glm::normalize(pos - target);
    cameraRight = glm::normalize(glm::cross(up, direction));
    cameraRight = glm::cross(direction, cameraRight);
    updateViewMatrix();
    updateProjectionMatrix();
}

Camera::~Camera()
{
    delete cameraUBO;
}

void Camera::setCameraPosition(const glm::vec3& newPos)
{
    pos = newPos;
    cameraUBO->UpdateData(&pos, sizeof(glm::vec3), sizeof(glm::mat4));
    if (lockTarget)
    {
        direction = glm::normalize(target - pos);
        updateCameraVectors();
    }
}

void Camera::setCameraDirection(const glm::vec3& newDirection)
{
    lockTarget = false;
    direction = glm::normalize(newDirection);
    updateCameraVectors();
}

void Camera::setCameraLookAt(const std::optional<glm::vec3>& newTarget)
{
    if (!newTarget.has_value())
    {
        // 取消锁定
        lockTarget = false;
    }
    else
    {
        // 锁定到新目标
        target = newTarget.value();
        lockTarget = true;
        direction = glm::normalize(target - pos);
        updateCameraVectors();
    }
}

void Camera::setPerspective(float newFov, float newAspectRatio, float newNearPlane, float newFarPlane)
{
    fov = newFov;
    aspectRatio = newAspectRatio;
    nearPlane = newNearPlane;
    farPlane = newFarPlane;
    orthographic = false;
    updateProjectionMatrix();
}

void Camera::setOrthographic(float left, float right, float bottom, float top, float newNearPlane, float newFarPlane)
{
    nearPlane = newNearPlane;
    farPlane = newFarPlane;
    orthographic = true;
    projectionMatrix = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
}

void Camera::setProjectionMode(bool useOrthographic)
{
    orthographic = useOrthographic;
    updateProjectionMatrix();
}

void Camera::processKey(bool Press_W, bool Press_A, bool Press_S, bool Press_D, float deltaTime)
{
    if (Press_W)
        setCameraPosition(pos + cameraSpeed * deltaTime * direction);
    if (Press_A)
        setCameraPosition(pos + cameraSpeed * deltaTime * cameraRight);
    if (Press_S)
        setCameraPosition(pos - cameraSpeed * deltaTime * direction);
    if (Press_D)
        setCameraPosition(pos - cameraSpeed * deltaTime * cameraRight);
    updateViewMatrix();
}

void Camera::updateViewMatrix()
{
    // 使用 pos、direction、cameraUp 计算视图矩阵
    viewMatrix = glm::lookAt(pos, pos + direction, cameraUp);
    updataViewProjectionMatrix();
}

void Camera::updateCameraVectors()
{
    // 更新摄像机的方向和上轴
    cameraRight = glm::normalize(glm::cross(up, direction));
    cameraUp = glm::normalize(glm::cross(direction, cameraRight));
    updateViewMatrix();
}

void Camera::updateProjectionMatrix()
{
    if (orthographic)
    {
        // Update orthographic projection (default bounds for simplicity)
        projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
    }
    else
    {
        // Update perspective projection
        projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    }
    updataViewProjectionMatrix();
}

void Camera::updataViewProjectionMatrix()
{
    viewProjectionMatrix = projectionMatrix * viewMatrix;
    cameraUBO->UpdateData(&viewProjectionMatrix, sizeof(glm::mat4), 0);
}
