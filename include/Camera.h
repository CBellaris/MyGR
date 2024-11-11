#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
private:
    glm::vec3 pos;
    glm::vec3 target;
    glm::vec3 direction;
    glm::vec3 up;
    glm::vec3 cameraRight;

    glm::mat4 view;

public:
    Camera();
    ~Camera();

    void setCameraPosition(const glm::vec3& pos);
    void setCameraDirection(const glm::vec3& direction);
    void setCameraLookAt(const glm::vec3& target);

    inline const glm::mat4& getViewMatrix() const {return view;}
};