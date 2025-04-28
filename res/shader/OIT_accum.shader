#shader vertex
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in float tangentW;

uniform mat4 modelMatrix;
layout(std140, binding = 0) uniform CameraUBO {
    mat4 viewProjectionMatrix;
    vec3 uViewPos;
};

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = viewProjectionMatrix * modelMatrix * vec4(aPos, 1.0);
}


#shader fragment
#version 460 core
layout (location = 0) out vec4 accumColor;
layout (location = 1) out float accumAlpha;

in vec2 TexCoords;

uniform sampler2D texture_diffuse0;


void main() {
    // 采样透明物体颜色
    vec4 color = texture(texture_diffuse0, TexCoords);
    float alpha = color.a;

    // 计算权重（可调整）
    float weight = max(alpha, 0.01);

    // 加权累积
    accumColor = vec4(color.rgb * alpha * weight, alpha * weight);
    accumAlpha = alpha * weight;
}
