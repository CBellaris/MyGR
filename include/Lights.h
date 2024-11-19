#include <glm/glm.hpp>
#include "Mesh.h"
#include <vector>

struct Light
{
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    int visible;
};

class Lights
{
private:
    Mesh* lightModel;
    std::vector<Light> lights;

public:
    Lights();
    ~Lights();

    void addLight(glm::vec3 pos = glm::vec3(1.0f), glm::vec3 color = glm::vec3(1.0f), float intensity = 1.0f, int visible = true);
    void ShowLightModel();

private:
    void update();
};