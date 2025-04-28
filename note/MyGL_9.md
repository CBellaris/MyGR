# 阴影、法线和高度贴图
## 阴影
直接先实现一个多光源的阴影系统，PCF和CSM放在后面的章节再来优化。对于平行光，使用普通的阴影贴图，对于点光源，使用立方体阴影贴图，然后因为需要在着色器中预留阴影贴图的槽位，所以需要规定每种光源的个数上限，目前暂定为每种3个。主要的实现方法在Light类当中：

首先在初始化添加灯光时，为每个光源创建对应的阴影贴图：
```cpp
// 根据灯光类型初始化阴影贴图
void initDirectionalShadowMap(ShadowMapInfo &shadowInfo);
void initPointShadowMap(ShadowMapInfo &shadowInfo);
```
这里为每个光源都单独创建了一个FBO，其实有些浪费，但为了图方便，就先这样。然后关键就在`bakeShadows()`函数当中：
```cpp
// 烘焙阴影贴图
void bakeShadows(Shader* shadowMapShader_directionalLight, Shader* shadowMapShader_pointLight, std::vector<std::shared_ptr<Model>>& models);
```
每个渲染循环中，都会调用这个函数（其实只有在物体和灯光改变时才需要重新烘焙，但这个逻辑比较难实现，先不做优化），使用两个对应的着色器，为平行光和点光源烘焙阴影贴图，具体方法和LearnOpenGL中阴影部分的内容基本一致

然后在`void bind_shadow(Shader* shader)`函数中，为需要使用阴影贴图的着色器设置各种所需的变量，这里先只对不透明物体，也就是主shader启用阴影功能，所以可以在`Basic.shader`中看到新加的一些变量：
```cpp
uniform mat4 lightProjection;   // 用于平行光阴影的正交投影矩阵
// 点光源立方体阴影所需的远剪裁面
uniform float farPlane;
uniform sampler2D shadowMaps[3];
uniform samplerCube shadowCubeMaps[3];
```
这里对于平行光，选择让它们共用一个正交透视矩阵，然后在片段着色器中计算FragPosLightSpace矩阵

然后我们新增了两个函数用于计算阴影：
```cpp
// 计算平行光阴影因子，传入光的属性、当前法线以及阴影贴图索引
float ComputeDirectionalShadow(Light light, int index, vec3 norm);
// 计算点光源阴影因子，使用立方体阴影贴图，传入光的属性和贴图索引
float ComputePointShadow(Light light, int index);
```

## 法线贴图和高度贴图
这两种贴图实现的核心即切线空间，需要先在Mesh类中添加对切线的支持。首先向Vertexdata中添加两个顶点数据：
```cpp
glm::vec3 Tangent;   // 切线
float tangentW;      // 切线方向标志位（+1 或 -1）
```
注意我们其实不需要在顶点中存储副切线，只需要在着色器中使用切线和法线做叉乘就可以得到副切线，但是这样副切线有正负两个可选的方向，用一个方向标志位来决定正反，一般这个标志位可以从Assimp读取得到，不过大部分时候将其设置为1即可正常工作

```cpp
// 切线计算函数
void calculateTangents(
    const std::vector<Vertexdata>& vertices, const std::vector<unsigned int>& indices, std::vector<glm::vec3>& tangents);
```
这个函数用于手动计算切线，我们手工定义的一些简单几何体例如立方体等，就可以用这个函数计算切线，然后是在 `set_mesh()` 函数中，可以选择是否计算切线，当使用Assimp读取模型时，直接读入Assimp计算好的切线，而自定义的几何体，则手动计算

然后在顶点着色器中计算出TBN矩阵：
```cpp
mat3 normalMatrix = mat3(modelMatrix);
normalMatrix = inverse(transpose(normalMatrix));

// 使用格拉姆-施密特正交化方法计算切线空间矩阵TBN
vec3 T = normalize(vec3(modelMatrix * vec4(tangent, 0.0)));
vec3 N = normalize(normalMatrix * aNormal); 
// re-orthogonalize T with respect to N
T = normalize(T - dot(T, N) * N);
// then retrieve perpendicular vector B with the cross productof T and N
vec3 B = cross(T, N) * tangentW; // 使用 tangentW 修正副切线方向
vs_out.TBN = mat3(T, B, N);
```

使用**转置TBN**矩阵将观察方向转换到切线空间，用于计算高度贴图带来的纹理坐标偏移：
```cpp
mat3 TBN_T = transpose(fs_in.TBN); // 转置用于世界空间 → 切线空间
vec3 tangentViewPos = TBN_T * fs_in.viewPos;
vec3 tangentFragPos = TBN_T * fs_in.FragPos;
vec3 tangentViewDir = normalize(tangentViewPos - tangentFragPos);
vec2 shiftTexCoord = ParallaxMapping(fs_in.TexCoord,  tangentViewDir);
```
然后使用**TBN**矩阵将切线空间采样的法向量转换至世界空间，来计算光线：
```cpp
vec3 tangentNormal = texture(texture_normal0, shiftTexCoord).rgb;
tangentNormal = normalize(tangentNormal * 2.0 - 1.0);
vec3 worldNormal = normalize(fs_in.TBN * tangentNormal);
```

然后我对Texture类做了比较多优化，现在`unsigned char* Texture::read_image(TextureImage& image)`函数支持读取.tga类型贴图，有些高度贴图会选择使用这个格式存储。并且支持管理每个类型贴图的上限，目前其实每种贴图不会大于1张，所以上限为1。

重点是我们需要为没有的纹理设置默认纹理，防止预期外的错误，在`void Texture::initialize_default_textures()`函数中：
```cpp
// 创建默认贴图
unsigned char grayPixel[3] = {128, 128, 128}; // 半灰
unsigned char bluePixel[3] = {128, 128, 255};     // 全蓝
unsigned char heightPixel[1] = {0};         // 用于 Height（单通道）
```
为反光贴图设置半灰，也可以根据需求调整。法线贴图要注意采样时的[0,1]->[-1,1]的映射，所以这里实际上设置了朝z轴的默认法向量。然后是高度贴图，设置为0，即没有高度偏移

### 未来优化
顶点数据现在是：
```cpp
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in float tangentW;
```
即使有些着色器没有用到一些数据，为了兼容绘制，依旧需要让所有的着色器都定义这些数据。其实这样的妥协做法目前不会带来很多性能负担，因为切换VBO才是主要的性能开销。不过我们依旧可以选择切换VAO的配置或者对一些着色器使用专门的VBO来进一步优化

## 全局变量管理
目前一些全局变量例如屏幕分辨率，我只是简单的存储在Config.h中，考虑到越来越多的全局设置和后面要加入的GUI，是时候将这些变量管理起来。现在的要求是，其他类可以方便的访问这些全局设置，并且可以在运行时被修改，使用单例类就比较合适。

首先安装一个JSON读取/写入库：https://github.com/nlohmann/json
只需要single_include/nlohmann/json.hpp即可，但我个人的习惯还是clone下所有的项目源文件，放在./vender/json下

然后创建GlobalSettings.h：
```cpp
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class GlobalSettings {
public:
    static GlobalSettings& Instance();

    void SetBool(const std::string& key, bool value);
    void SetFloat(const std::string& key, float value);
    void SetInt(const std::string& key, int value);

    bool GetBool(const std::string& key) const;
    float GetFloat(const std::string& key) const;
    int GetInt(const std::string& key) const;

    // JSON 加载/保存接口
    bool LoadFromFile(const std::string& filename);
    bool SaveToFile(const std::string& filename) const;

private:
    GlobalSettings() = default;

    std::unordered_map<std::string, bool> boolSettings;
    std::unordered_map<std::string, float> floatSettings;
    std::unordered_map<std::string, int> intSettings;
};
```
创建一个res/settings.json来保存全局设置，在程序开始时读取设置，结束时保存即可，后续可以添加接口用于运行时修改变量

## 构建
构建本节的代码存档，可以看到多光源阴影被渲染，你可以加载不同的纹理贴图来查看法线和高度贴图的效果，当应用上高度纹理后，两个面的接缝处会有渲染错误，这个比较难优化，高度贴图本身应用在一个面上面效果比较好

<img src="assets\C9_0.png" style="zoom:50%;" />
<img src="assets\C9_1.png" style="zoom:50%;" />

还可以按0-5切换渲染模式，0：默认模式；1：世界空间法线；2：切线空间法线；3：视线方向；4：线性深度；5. 偏移可视化。这些可视化可以帮助debug，在后面的开发中会越来越重要：

<img src="assets\C9_2.png" style="zoom:50%;" />
<img src="assets\C9_3.png" style="zoom:50%;" />
<img src="assets\C9_4.png" style="zoom:50%;" />
<img src="assets\C9_5.png" style="zoom:50%;" />

## 错误修正
你可能已经发现了，在显示高度贴图时，立方体的其中3个面效果比较奇怪，这是因为这里手工定义的立方体顶点数据还比较原始，没有考虑**顶点的顺序**和纹理坐标的**UV顺序**，顶点顺序的错误可以通过开启面剔除来查看，OpenGL规定按逆时针定义的点为正面。在修复手工定义的立方体顶点数据时，可以在纸面上分别正对每个面画出其6个顶点，标注点的顺序

其次是UV顺序，上文中的 tangentW=1 实际上指的就是顶点顺序和UV顺序同向，当顶点按逆时针定义时，UV也应该按逆时针定义：
```cpp
Vertexdata(-1.0f, -1.0f, 1.0f,   0.0f, 0.0f, 1.0f,    0.0f, 0.0f),
Vertexdata( 1.0f, -1.0f, 1.0f,   0.0f, 0.0f, 1.0f,    1.0f, 0.0f),
Vertexdata( 1.0f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f,    1.0f, 1.0f),
Vertexdata( 1.0f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f,    1.0f, 1.0f),
Vertexdata(-1.0f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f),
Vertexdata(-1.0f, -1.0f, 1.0f,   0.0f, 0.0f, 1.0f,    0.0f, 0.0f), 
```
这里是一组错误的顶点数据，UV按照顺时针定义了：(0,0)->(1,0)->(1,1)->(0,1)->(0,0)

在后面的代码存档中，这里被修改过后，我们的高度贴图就渲染正确了