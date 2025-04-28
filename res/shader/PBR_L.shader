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
} vs_out;

layout(std140, binding = 0) uniform CameraUBO {
    mat4 viewProjectionMatrix;
    vec3 uViewPos;
};

void main()
{
    vs_out.viewPos = uViewPos;
    vs_out.TexCoord = aTexCoord;
    gl_Position = vec4(aPos.xy, 0.0, 1.0);
}


#shader fragment
#version 460 core

in VS_OUT {
    vec3 viewPos;
    vec2 TexCoord;
} fs_in;

// G-buffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMetallicRoughnessAO;
uniform sampler2D ssao;

uniform samplerCube texture_prefilterMap;
uniform sampler2D   texture_brdfLUT;  

// 设置调试模式
uniform int debugMode;

out vec4 FragColor;

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

void main()
{
    const float MAX_REFLECTION_LOD = 4.0;

    // 获取输入
    vec3 FragPos = texture(gPosition, fs_in.TexCoord).rgb;
    vec3 Normal = texture(gNormal, fs_in.TexCoord).rgb;
    vec3 Albedo = texture(gAlbedo, fs_in.TexCoord).rgb;
    float Metallic = texture(gMetallicRoughnessAO, fs_in.TexCoord).r;
    float Roughness = texture(gMetallicRoughnessAO, fs_in.TexCoord).g;
    float AO = texture(gMetallicRoughnessAO, fs_in.TexCoord).b;
    float AmbientOcclusion = texture(ssao, fs_in.TexCoord).r;   // 这个是SSAO

    vec3 V = normalize(fs_in.viewPos - FragPos);
    vec3 N = normalize(Normal);
    vec3 R = reflect(-V, N); 

    // 计算KS和KD
    vec3 F0 = vec3(0.04); // 默认F0值
    F0 = mix(F0, Albedo, Metallic); // 线性插值计算F0
    vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, Roughness); // 计算Fresnel-Schlick近似值
    vec3 kD = 1.0 - kS;
    kD *= (1.0 - Metallic); 

    // 采样得到漫反射的积分值
    vec3 irradiance = textureLod(texture_prefilterMap, N, MAX_REFLECTION_LOD).rgb;
    vec3 diffuse    = irradiance * Albedo;

    // 采样得到镜面反射的积分值
    vec3 prefilteredColor = textureLod(texture_prefilterMap, R,  Roughness * MAX_REFLECTION_LOD).rgb;   
    vec2 envBRDF  = texture(texture_brdfLUT, vec2(max(dot(N, V), 0.0), Roughness)).rg;
    vec3 specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y);

    vec3 ambient = (kD * diffuse + specular) * AO; 
    ambient = ambient * AmbientOcclusion; // 乘以环境光遮蔽因子

    vec3 color = ambient;
    color = color / (color + vec3(1.0)); // tone mapping
    color = pow(color, vec3(1.0/2.2));   // gamma correction
    FragColor = vec4(color, 1.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   