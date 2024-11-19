#shader vertex
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;



out vec2 TexCoord;
uniform mat4 aTransMatrix;

void main()
{
    gl_Position = aTransMatrix * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}


#shader fragment
#version 460 core
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoord);
}