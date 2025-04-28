#shader vertex
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in float tangentW;

uniform mat4 modelMatrix;
layout(std140, binding = 0) uniform CameraUBO {
    mat4 viewProjectionMatrix;
    vec3 uViewPos;
};

void main()
{
    gl_Position = viewProjectionMatrix * modelMatrix * vec4(aPos, 1.0);
}


#shader fragment
#version 460 core
out vec4 FragColor;

uniform vec3 outlineColor;

void main()
{
    FragColor = vec4(outlineColor, 1.0f);
}