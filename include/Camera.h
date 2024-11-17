#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>

class Camera
{
private:
    glm::vec3 pos;
    glm::vec3 target;
    glm::vec3 direction;
    glm::vec3 up;
    glm::vec3 cameraRight;
    glm::vec3 cameraUp;
    bool lockTarget; 

    glm::mat4 viewMatrix;

    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;
    bool orthographic;

    glm::mat4 projectionMatrix;
    glm::mat4 viewProjectionMatrix;

    float cameraSpeed;

public:
    Camera();
    ~Camera();

    void setCameraPosition(const glm::vec3& pos);
    void setCameraDirection(const glm::vec3& direction);
    void setCameraLookAt(const std::optional<glm::vec3>& newTarget);

    void setPerspective(float fov, float aspectRatio, float nearPlane, float farPlane);
    void setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    void setProjectionMode(bool useOrthographic);

    inline const glm::mat4& getViewMatrix() const {return viewMatrix;}
    inline const glm::mat4& getViewProjectionMatrix() const {return viewProjectionMatrix;}

    void processKey(bool Press_W, bool Press_A, bool Press_S, bool Press_D, float deltaTime);
private:
    void updateViewMatrix();
    void updateCameraVectors();
    void updateProjectionMatrix();
    void updataViewProjectionMatrix();
};