#shader vertex
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in float tangentW;

out vec3 TexCoords;

layout(std140, binding = 0) uniform CameraUBO {
    mat4 viewProjectionMatrix;
    vec3 uViewPos;
};

void main()
{
    TexCoords = aPos;
    
    // 移除 view 矩阵的平移部分
    mat4 viewProjNoTranslation = viewProjectionMatrix;
    viewProjNoTranslation[3] = vec4(0.0, 0.0, 0.0, 1.0); // 清除平移

    gl_Position = viewProjNoTranslation * vec4(aPos, 1.0);
}


#shader fragment
#version 460 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube texture_cubemap;

void main()
{    
    FragColor = texture(texture_cubemap, TexCoords);

    // 手动设置深度值
    gl_FragDepth = 0.9999; // 设置为远平面深度
}