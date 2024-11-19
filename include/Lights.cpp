#include "Lights.h"

Lights::Lights()
{
}

Lights::~Lights()
{
    delete lightModel;
}

void Lights::addLight(glm::vec3 pos, glm::vec3 color, float intensity, int visible)
{
    Light light= {
        pos,
        color,
        intensity,
        visible
    };
    lights.push_back(light);
}

void Lights::ShowLightModel()
{
    
}

void Lights::update()
{
    lightModel->setPosition(pos);
}
