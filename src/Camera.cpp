#include "Camera.h"

Camera::Camera() : pos(glm::vec3(0.0f, 0.0f, 1.0f)), target(0.0f), up(glm::vec3(0.0f, 1.0f, 0.0f))
{
    direction = glm::normalize(pos - target);
    cameraRight = glm::normalize(glm::cross(up, direction));
    cameraRight = glm::cross(direction, cameraRight);
}

Camera::~Camera()
{
}

//------------------------------------------
#include <optional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
private:
    glm::vec3 pos;           // 摄像机位置
    glm::vec3 target;        // 锁定的目标位置
    glm::vec3 direction;     // 摄像机方向
    glm::vec3 up;            // 世界空间中的上向量
    glm::vec3 cameraRight;   // 摄像机的右向量
    glm::vec3 cameraUp;      // 摄像机的上向量

    glm::mat4 view;          // 视图矩阵

    bool lockTarget;         // 判断是否锁定到目标

    void updateViewMatrix()
    {
        // 使用 pos、direction、cameraUp 计算视图矩阵
        view = glm::lookAt(pos, pos + direction, cameraUp);
    }

    void updateCameraVectors()
    {
        // 更新摄像机的方向和上轴
        if (lockTarget)
        {
            direction = glm::normalize(target - pos);
        }
        cameraRight = glm::normalize(glm::cross(up, direction));
        cameraUp = glm::normalize(glm::cross(direction, cameraRight));
        updateViewMatrix();
    }

public:
    Camera()
        : pos(glm::vec3(0.0f, 0.0f, 1.0f)), target(glm::vec3(0.0f)), up(glm::vec3(0.0f, 1.0f, 0.0f)), lockTarget(false)
    {
        direction = glm::normalize(pos - target);
        cameraRight = glm::normalize(glm::cross(up, direction));
        cameraUp = glm::normalize(glm::cross(direction, cameraRight));
        updateViewMatrix();
    }

    ~Camera() {}

    void setCameraPosition(const glm::vec3& newPos)
    {
        pos = newPos;
        if (!lockTarget)
        {
            direction = glm::normalize(pos - target);
        }
        updateCameraVectors();
    }

    void setCameraDirection(const glm::vec3& newDirection)
    {
        if (!lockTarget)
        {
            direction = glm::normalize(newDirection);
            updateCameraVectors();
        }
    }

    void setCameraLookAt(const std::optional<glm::vec3>& newTarget)
    {
        if (!newTarget.has_value())
        {
            // 取消锁定
            lockTarget = false;
        }
        else
        {
            // 锁定到新目标
            target = *newTarget;
            lockTarget = true;
            direction = glm::normalize(target - pos);
            updateCameraVectors();
        }
    }

    inline const glm::mat4& getViewMatrix() const { return view; }
};
