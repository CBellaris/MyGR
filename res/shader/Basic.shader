#shader vertex
#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}


#shader fragment
#version 460 core
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 color_b;
uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoord);
}