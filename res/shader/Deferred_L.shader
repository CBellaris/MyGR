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
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssao;

// 灯光相关变量
struct Light {
    vec3 position;
    vec3 direction;
    vec3 color;
    vec3 intensity;

    float constant;
    float linear;
    float quadratic;

    int visibility;
    int isDirectional;

    float padding1;
    float padding2;
    float padding3;
};
layout(std430, binding = 1) buffer LightBuffer {
    Light lights[];
};
uniform int numLights;

uniform mat4 lightProjection;   // 用于平行光阴影的正交投影矩阵
// 点光源立方体阴影所需的远剪裁面
uniform float farPlane;
uniform sampler2D shadowMaps[3];
uniform samplerCube shadowCubeMaps[3];

// 设置调试模式
// 0：默认模式；1：世界空间法线；2：视线方向；3：线性深度; 4.环境光遮蔽
uniform int debugMode;

out vec4 FragColor;

// 计算平行光阴影因子，传入光的属性、当前法线以及阴影贴图索引
float ComputeDirectionalShadow(Light light, int index, vec3 norm, vec3 FragPos);
// 计算点光源阴影因子，使用立方体阴影贴图，传入光的属性和贴图索引
float ComputePointShadow(Light light, int index, vec3 FragPos);
// 光照计算
void ComputeLighting(vec3 lightColor, vec3 lightDir, vec3 viewDir, vec3 norm, vec3 intensity, vec3 Albedo, float Specular, out vec3 ambient, out vec3 diffuse, out vec3 specular);
// 针对点光源：先做基本光照，再根据距离计算衰减，并结合立方体阴影
void ComputePointLight(Light light, vec3 norm, vec3 viewDir, int shadowIndex, vec3 FragPos, vec3 Albedo, float Specular, out vec3 ambient, out vec3 diffuse, out vec3 specular);
// 针对平行光：直接计算光照，并结合常规阴影（这里镜面分量直接去除阴影影响）
void ComputeDirectionalLight(Light light, vec3 norm, vec3 viewDir, int shadowIndex, vec3 FragPos, vec3 Albedo, float Specular, out vec3 ambient, out vec3 diffuse, out vec3 specular);

mat4 lookAt(vec3 eye, vec3 center, vec3 up);

void main()
{
    vec3 FragPos = texture(gPosition, fs_in.TexCoord).rgb;
    vec3 Normal = texture(gNormal, fs_in.TexCoord).rgb;
    vec3 Albedo = texture(gAlbedoSpec, fs_in.TexCoord).rgb;
    float Specular = texture(gAlbedoSpec, fs_in.TexCoord).a;
    float Depth = texture(gPosition, fs_in.TexCoord).a; // 深度信息存储在gPosition.a中
    float AmbientOcclusion = texture(ssao, fs_in.TexCoord).r;

    vec3 worldViewDir = normalize(fs_in.viewPos - FragPos);

    // Debug 分支
    if (debugMode == 1) {
        // 世界空间法线
        FragColor = vec4(Normal * 0.5 + 0.5, 1.0);
        return;
    } else if (debugMode == 2) {
        // 视线方向可视化
        FragColor = vec4(normalize(worldViewDir) * 0.5 + 0.5, 1.0);
        return;
    } else if (debugMode == 3) {
        FragColor = vec4(vec3(Depth), 1.0); // 使用深度值作为线性深度
        return;
    }  else if (debugMode == 4) {
        FragColor = vec4(vec3(AmbientOcclusion), 1.0); // 使用环境光遮蔽值
        return;
    }

    vec3 norm = Normal;
    vec3 totalAmbient = vec3(0.0);
    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);

    // 分别计数以选择正确的阴影贴图
    int dirShadowIndex = 0;
    int pointShadowIndex = 0;

    for (int i = 0; i < numLights; i++) {
        Light light = lights[i];
        if (light.visibility == 0) continue;

        vec3 ambient, diffuse, specular;
        if (light.isDirectional == 1) {
            ComputeDirectionalLight(light, norm, worldViewDir, dirShadowIndex, FragPos, Albedo, Specular, ambient, diffuse, specular);
            dirShadowIndex++;  // 下一个平行光用下一个阴影贴图槽
        } else {
            ComputePointLight(light, norm, worldViewDir, pointShadowIndex, FragPos, Albedo, Specular, ambient, diffuse, specular);
            pointShadowIndex++;  // 下一个点光源用下一个立方体阴影贴图槽
        }
        totalAmbient += ambient;
        totalDiffuse += diffuse;
        totalSpecular += specular;
    }

    // 环境光上限设为 0.8
    totalAmbient = min(totalAmbient, vec3(0.8)) * AmbientOcclusion; // 乘以环境光遮蔽因子
    vec3 resultColor = totalAmbient + totalDiffuse + totalSpecular;
    FragColor = vec4(resultColor, 1.0);
}

// 计算平行光阴影因子，传入光的属性、当前法线以及阴影贴图索引
float ComputeDirectionalShadow(Light light, int index, vec3 norm, vec3 FragPos)
{
    // 构造光的观察矩阵（这里假定 up 向量为 (0,1,0)）
    vec3 up = vec3(0.0, 1.0, 0.0);
    mat4 lightView = lookAt(light.position, light.position + light.direction, up);
    mat4 lightSpaceMatrix = lightProjection * lightView;
    vec4 FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    // 透视除法，将坐标转换到 [0,1] 区间
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    // 超过远平面的点直接返回 0.0
    if(projCoords.z > 1.0){
        return 0.0;
    }
        
    // 如果在阴影贴图范围外，不计算阴影
    if(projCoords.x < 0.0 || projCoords.x > 1.0 ||
       projCoords.y < 0.0 || projCoords.y > 1.0)
       return 0.0;

    // 设定深度偏移（bias）以缓解自阴影问题
    float bias = max(0.05 * (1.0 - dot(norm, normalize(-light.direction))), 0.005);

    float closestDepth = texture(shadowMaps[index], projCoords.xy).r;

    // 简单的影子测试（此处可扩展为 PCF 滤波）
    float shadow = (projCoords.z - bias > closestDepth) ? 1.0 : 0.0;

    return shadow;
}

// 计算点光源阴影因子，使用立方体阴影贴图，传入光的属性和贴图索引
float ComputePointShadow(Light light, int index, vec3 FragPos)
{
    vec3 fragToLight = FragPos - light.position;
    float currentDepth = length(fragToLight);

    // 设定简单偏移防止自阴影
    float bias = 0.05;
    float closestDepth = texture(shadowCubeMaps[index], fragToLight).r;

    // 立方体阴影贴图采样返回 [0,1] 范围的深度值，需要乘以 farPlane 得到实际距离
    closestDepth *= farPlane;

    float shadow = (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
    return shadow;
}

// 光照计算
void ComputeLighting(vec3 lightColor, vec3 lightDir, vec3 viewDir, vec3 norm, vec3 intensity, vec3 Albedo, float Specular, out vec3 ambient, out vec3 diffuse, out vec3 specular) {
    float diff = max(dot(norm, lightDir), 0.0);
    
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    
    ambient  = intensity.x * lightColor * Albedo;
    diffuse  = intensity.y * lightColor * diff * Albedo;
    specular = intensity.z * lightColor * spec * Specular * vec3(1.0, 1.0, 1.0); // specular color is white
}

// 针对点光源：先做基本光照，再根据距离计算衰减，并结合立方体阴影
void ComputePointLight(Light light, vec3 norm, vec3 viewDir, int shadowIndex, vec3 FragPos, vec3 Albedo, float Specular, out vec3 ambient, out vec3 diffuse, out vec3 specular)
{
    vec3 lightDir = normalize(light.position - FragPos);
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    ComputeLighting(light.color, lightDir, viewDir, norm, light.intensity, Albedo, Specular, ambient, diffuse, specular);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    // 计算点光源阴影因子，并对漫反射和镜面贡献进行削弱
    float shadow = ComputePointShadow(light, shadowIndex, FragPos);
    diffuse *= (1.0 - shadow);
    specular *= (1.0 - shadow);
}

// 针对平行光：直接计算光照，并结合常规阴影（这里镜面分量直接去除阴影影响）
void ComputeDirectionalLight(Light light, vec3 norm, vec3 viewDir, int shadowIndex, vec3 FragPos, vec3 Albedo, float Specular, out vec3 ambient, out vec3 diffuse, out vec3 specular)
{
    vec3 lightDir = normalize(-light.direction);

    ComputeLighting(light.color, lightDir, viewDir, norm, light.intensity, Albedo, Specular, ambient, diffuse, specular);

    // 计算平行光的阴影因子
    float shadow = ComputeDirectionalShadow(light, shadowIndex, norm, FragPos);
    diffuse *= (1.0 - shadow);
    // 平行光一般不计算镜面反射阴影
    specular = vec3(0.0);
}

mat4 lookAt(vec3 eye, vec3 center, vec3 up) {
    vec3 f = normalize(center - eye); // forward
    vec3 s = normalize(cross(f, up)); // right
    vec3 u = cross(s, f);             // up (recomputed to be orthogonal)

    mat4 result = mat4(1.0);
    result[0][0] = s.x;
    result[1][0] = s.y;
    result[2][0] = s.z;

    result[0][1] = u.x;
    result[1][1] = u.y;
    result[2][1] = u.z;

    result[0][2] = -f.x;
    result[1][2] = -f.y;
    result[2][2] = -f.z;

    result[3][0] = -dot(s, eye);
    result[3][1] = -dot(u, eye);
    result[3][2] = dot(f, eye);
    
    return result;
}