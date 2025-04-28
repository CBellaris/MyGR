# PBR
这里只实现IBL看看效果，因为非光追管线想要PBR，还想兼容自定义光源，阴影等功能的话，路线太多太杂，所以后面不考虑继续完善非光追管线了，直接转向光追+PBR

只是实现基于图像光照的PBR就比较简单了，总共是下面几块：
## 预滤波环境贴图
其实这里就引入了重要性采样的思想，根据物体的粗糙度，仅对视线反射向量的锥形区域进行预积分，就提前计算好了渲染方程中的L项，当然这里L项可以单独计算是因为用到了一个近似等式，https://www.bilibili.com/video/BV1YK4y1T7yY 的p3中也有提到，这里的近似等式则要求BRDF项光滑或者积分区域较小，由于对BRDF的积分也是在一个小的锥形区域里，并且除开物体边缘，BRDF的值基本不会有特别剧烈的变化，所以我们可以认为这个近似等式是相对准确的

具体的计算放在了Texture类当中`GLuint Texture::prefilter_cubemap(GLuint cubemap, int resolution, int cubemap_resolution)`，我们之前就加载过HDRI贴图，在将其转换为立方体贴图后，使用这个函数进行预滤波

## 预积分BRDF
这里的理论推导在LearnOpenGL中比较详细了，仅补充一些小知识。这里单独分离菲涅尔项，除了形式上允许外，实际上就是在提前对 F0 = 0 和 F0 = 1 的两种极端情况进行预计算，然后运行时插值，从而节省存储与计算资源。

BRDF在渲染方程中是需要入射和观察角度作为变量的，则最少需要4个查询维度，将brdfLUT简化为只依赖 N⋅V 和 roughness，实际上可以想象到，这里的BRDF必须假设是径向对称的，也就是各项同性（与表面旋转无关）。如果想要拓展至各向异性的材料，则需要提升LUT的维度

具体的计算还是放在Texture类当中，调用`void Texture::add_preCal_CT_BRDF(int resolution)` 来创建一个基于Cook-Torrance的brdfLUT

## G-buffer Pass
PBR_G.shader与前面的延迟渲染G-buffer Pass基本相同，额外向G-buffer中存储`layout (location = 3) out vec3 gMetallicRoughnessAO;`即可

## Light Pass
PBR_L.shader也比较简单，首先引入两个预计算贴图：
```cpp
uniform samplerCube texture_prefilterMap;
uniform sampler2D   texture_brdfLUT;  
```
获取一堆需要的数据：
```cpp
// 获取输入
vec3 FragPos = texture(gPosition, fs_in.TexCoord).rgb;
vec3 Normal = texture(gNormal, fs_in.TexCoord).rgb;
vec3 Albedo = texture(gAlbedo, fs_in.TexCoord).rgb;
float Metallic = texture(gMetallicRoughnessAO, fs_in.TexCoord).r;
float Roughness = texture(gMetallicRoughnessAO, fs_in.TexCoord).g;
float AO = texture(gMetallicRoughnessAO, fs_in.TexCoord).b;
float AmbientOcclusion = texture(ssao, fs_in.TexCoord).r;   // 这个是SSAO
```
然后同样使用菲涅尔函数计算漫反射和镜面反射的比率，这里的菲涅尔和镜面反射中BRDF中的菲涅尔项不是一个东西，别搞混了：
```cpp
// 计算KS和KD
vec3 F0 = vec3(0.04); // 默认F0值
F0 = mix(F0, Albedo, Metallic); // 线性插值计算F0
vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, Roughness); // 计算Fresnel-Schlick近似值
vec3 kD = 1.0 - kS;
kD *= (1.0 - Metallic); 
```
这里采样漫反射时，并没有用一个单独的半球滤波环境贴图，直接使用为镜面反射生成的预滤波环境贴图的最高粗糙度挡位就可以，比较方便，效果也差不多（应该）：
```cpp
// 采样得到漫反射的积分值
vec3 irradiance = textureLod(texture_prefilterMap, N, MAX_REFLECTION_LOD).rgb;
vec3 diffuse    = irradiance * Albedo;
```
然后采样镜面反射的积分：
```cpp
// 采样得到镜面反射的积分值
vec3 prefilteredColor = textureLod(texture_prefilterMap, R,  Roughness * MAX_REFLECTION_LOD).rgb;   
vec2 envBRDF  = texture(texture_brdfLUT, vec2(max(dot(N, V), 0.0), Roughness)).rg;
vec3 specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y);
```
最后应用上AO和SSAO（目前PBR管线没法渲染复杂场景，所以其实没啥用），HDR和伽马修正，就ok了：
```cpp
vec3 ambient = (kD * diffuse + specular) * AO; 
ambient = ambient * AmbientOcclusion; // 乘以环境光遮蔽因子

vec3 color = ambient;
color = color / (color + vec3(1.0)); // tone mapping
color = pow(color, vec3(1.0/2.2));   // gamma correction
FragColor = vec4(color, 1.0);
```

最后的伽马修正非常重要，其实在G-Buffer Pass中采样贴图时，应该对漫反射贴图做反伽马修正，但这个东西和贴图作者本身有没有创建sRBG贴图有关系，所以应该根据视觉效果灵活修改

## 构建
就加载一个贴图看看效果，目前想直接使用我们的模型类导入带有PBR贴图的模型基本都会出错，主要是因为Assip对贴图的对应：
```cpp
Texture* texture = new Texture();
if (mesh->mMaterialIndex >= 0) {
    process_material_textures(material, aiTextureType_DIFFUSE, TextureType::Diffuse, texture, scene, this->directory); 
    process_material_textures(material, aiTextureType_METALNESS, TextureType::Metallic, texture, scene, this->directory);  
    process_material_textures(material, aiTextureType_DIFFUSE_ROUGHNESS, TextureType::Roughness, texture, scene, this->directory);  
    process_material_textures(material, aiTextureType_HEIGHT, TextureType::Height, texture, scene, this->directory);
    process_material_textures(material, aiTextureType_NORMALS, TextureType::Normal, texture, scene, this->directory);
    process_material_textures(material, aiTextureType_AMBIENT, TextureType::AO, texture, scene, this->directory);
}
```
因为软件和格式的各种不一致，没有一个统一的口径，有的老一点的模型，读进来aiTextureType_SHININESS对应粗糙度，基本上要一个个手动调整，很多时候在3D软件里都比较麻烦，就暂时不解决了

<img src="assets\C12_0.png" style="zoom:50%;" />