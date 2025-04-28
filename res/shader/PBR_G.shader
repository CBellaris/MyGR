#shader vertex
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in float tangentW;

out VS_OUT {
    vec3 FragPos; 
    vec2 TexCoord;
    vec3 viewPos;
    mat3 TBN; // 切线空间矩阵
} vs_out;

uniform mat4 modelMatrix;
layout(std140, binding = 0) uniform CameraUBO {
    mat4 viewProjectionMatrix;
    vec3 uViewPos;
};

void main()
{
    mat3 normalMatrix = mat3(modelMatrix);
    normalMatrix = inverse(transpose(normalMatrix));

    // 使用格拉姆-施密特正交化方法计算切线空间矩阵TBN
    vec3 T = normalize(vec3(modelMatrix * vec4(tangent, 0.0)));
    vec3 N = normalize(normalMatrix * aNormal); 
    // re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);
    // then retrieve perpendicular vector B with the cross product of T and N
    vec3 B = cross(T, N) * tangentW; // 使用 tangentW 修正副切线方向
    vs_out.TBN = mat3(T, B, N);

    gl_Position = viewProjectionMatrix * modelMatrix * vec4(aPos, 1.0);
    vs_out.FragPos = vec3(modelMatrix * vec4(aPos, 1.0));
    vs_out.TexCoord = aTexCoord;
    vs_out.viewPos = uViewPos;
}

#shader fragment
#version 460 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gMetallicRoughnessAO;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoord;
    vec3 viewPos;
    mat3 TBN; // 切线空间矩阵
} fs_in;

// 材质，按照规则命名
uniform sampler2D texture_diffuse0;
uniform sampler2D texture_normal0; 
uniform sampler2D texture_metallic0;
uniform sampler2D texture_roughness0;
uniform sampler2D texture_ao0;
uniform sampler2D texture_height0;

uniform float height_scale;

out vec4 FragColor;

// 平行映射函数
vec2 ParallaxMapping(vec2 texCoord, vec3 viewDir);

void main()
{
    mat3 TBN_T = transpose(fs_in.TBN); // 转置用于世界空间 → 切线空间
    vec3 tangentViewPos = TBN_T * fs_in.viewPos;
    vec3 tangentFragPos = TBN_T * fs_in.FragPos;
    vec3 tangentViewDir = normalize(tangentViewPos - tangentFragPos);
    vec2 shiftTexCoord = ParallaxMapping(fs_in.TexCoord,  tangentViewDir);

    vec3 tangentNormal = texture(texture_normal0, shiftTexCoord).rgb;
    tangentNormal = normalize(tangentNormal * 2.0 - 1.0);
    vec3 worldNormal = normalize(fs_in.TBN * tangentNormal);

    // 实际片段深度
    float linearDepth = length(fs_in.viewPos - fs_in.FragPos);
    // 存储第一个G缓冲纹理中的片段位置向量
    gPosition = vec4(fs_in.FragPos, linearDepth); // 将深度信息存储在gPosition.a中
    // 存储第二个G缓冲纹理中的片段法线向量(世界空间)
    gNormal = worldNormal;
    // 和漫反射对每个逐片段颜色
    gAlbedo.rgb = texture(texture_diffuse0, shiftTexCoord).rgb;
    // 存储金属度、粗糙度和环境光遮蔽到gMetallicRoughnessAO的分量
    gMetallicRoughnessAO.r = texture(texture_metallic0, shiftTexCoord).r; // 金属度
    gMetallicRoughnessAO.g = texture(texture_roughness0, shiftTexCoord).r; // 粗糙度
    gMetallicRoughnessAO.b = texture(texture_ao0, shiftTexCoord).r; // 环境光遮蔽
}

vec2 ParallaxMapping(vec2 texCoord, vec3 viewDir)
{ 
    // 视角越倾斜，采样层数越多
    const float minLayers = 8;
    const float maxLayers = 32;
    float ndotv = clamp(dot(vec3(0.0, 0.0, 1.0), normalize(viewDir)), 0.0, 1.0);
    float numLayers = mix(maxLayers, minLayers, ndotv);
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy * height_scale; 
    vec2 deltaTexCoords = P / numLayers;

    // get initial values
    vec2  currentTexCoords     = texCoord;
    float currentDepthMapValue = texture(texture_height0, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue)
    {
        currentTexCoords = clamp(currentTexCoords, vec2(0.001), vec2(0.999)); // 防止越界

        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(texture_height0, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
        
    }
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(texture_height0, prevTexCoords).r - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;

}
