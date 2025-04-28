#shader vertex
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in float tangentW;

out VS_OUT {
    vec3 viewPos;
    vec2 TexCoord;
    mat4 projection;
} vs_out;

layout(std140, binding = 0) uniform CameraUBO {
    mat4 viewProjectionMatrix;
    vec3 uViewPos;
};

void main()
{
    vs_out.viewPos = uViewPos;
    vs_out.TexCoord = aTexCoord;
    vs_out.projection = viewProjectionMatrix;
    gl_Position = vec4(aPos.xy, 0.0, 1.0);
}


#shader fragment
#version 460 core

in VS_OUT {
    vec3 viewPos;
    vec2 TexCoord;
    mat4 projection;
} fs_in;

out float FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;

uniform sampler2D texture_noise; // 噪声纹理

uniform vec3 samples[64];

// 屏幕的平铺噪声纹理会根据屏幕分辨率除以噪声大小的值来决定
uniform vec2 noiseScale;
uniform float ssaoStrengh; // 亮度调整指数

void main()
{
    vec3 fragPos = texture(gPosition, fs_in.TexCoord).xyz;
    float fragDepth = texture(gPosition, fs_in.TexCoord).a;
    vec3 normal = texture(gNormal, fs_in.TexCoord).rgb;
    vec3 randomVec = texture(texture_noise, fs_in.TexCoord * noiseScale).xyz;

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float radius = 1.0; // 采样半径
    int kernelSize = 64; // 采样数量
    float bias = 0.025; // 深度偏移量
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // 获取样本位置
        vec3 sample_unit = TBN * samples[i]; // 切线->世界空间
        sample_unit = fragPos + normal * bias + sample_unit * radius;
        float sample_depth = length(sample_unit - fs_in.viewPos); // 计算实际采样点深度

        // 获取实际片段深度
        vec4 sampleClip = fs_in.projection * vec4(sample_unit, 1.0); // 世界->裁剪空间
        sampleClip.xyz /= sampleClip.w; // 透视除法
        sampleClip.xyz = sampleClip.xyz * 0.5 + 0.5; // 归一化到[0,1]
        float sampleDepth = texture(gPosition, sampleClip.xy).a;

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragDepth - sampleDepth));
        occlusion += (sampleDepth <= sample_depth ? 1.0 : 0.0) * rangeCheck;   
    }
    occlusion = 1.0 - (occlusion / kernelSize); // 归一化


    FragColor = pow(occlusion, ssaoStrengh);
}
