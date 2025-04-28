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

uniform sampler2D accum_texture;
uniform sampler2D alpha_texture;

void main() {
    vec4 accum = texture(accum_texture, gl_FragCoord.xy / vec2(textureSize(accum_texture, 0)));
    float alpha = texture(alpha_texture, gl_FragCoord.xy / vec2(textureSize(alpha_texture, 0))).r;

    // 计算最终颜色
    if (alpha > 1e-6) {
        FragColor = vec4(accum.rgb / alpha, alpha);
    } else {
        FragColor = vec4(0.0);
    }
}
