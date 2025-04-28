#shader vertex
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in float tangentW;

void main() {
    gl_Position = vec4(aPos.xy, 0.0, 1.0);
}

#shader fragment
#version 460 core

out vec4 FragColor;

uniform sampler2D texture0;

void main() {
    vec2 col = texture(texture0, gl_FragCoord.xy/vec2(textureSize(texture0, 0))).rg;
    FragColor = vec4(col, 0.0,  1.0);
}
