# 渲染器
在这一章之前，我们基本将LearnOpenGL中的入门部分实现完了，在开始光照部分前，还有一个非常重要的操作，没错，从这一章的第一部分（光照/颜色）开始，我们的场景需要两个着色器，一个用于渲染物体，一个用于渲染灯本身，当然，如果只是实现这个功能，并没有什么难度，但如果想将其系统性的管理，其实非常棘手

这里的核心有两点：
1. 切换着色器绑定是一个高消耗的操作，在修改着色器的全局变量（目前有摄像机矩阵，模型矩阵和灯光颜色等）前和渲染物体（假设有多个灯光和物体）前，我们都需要绑定对应的着色器
2. 一个摄像机可以对应多个着色器，一个着色器可以渲染多个物体

其中部分问题可以用Uniform Buffer Object (UBO)来解决，但先假设我们并不知道这一点，想想上面的操作，摄像机矩阵，模型矩阵和灯光颜色等都维护在不同的类和多个实例中，而且还有不同的对应关系，如果非常随意的在这些实例中处理着色器全局变量和渲染，那么会导致非常频繁的切换着色器绑定，这样是不行的，一个**合理的渲染顺序**应该是：
1. 按 Shader 分组
2. 每个 Shader 只切换一次
3. 在绑定 Shader 之后，更新该 Shader 需要的 Uniform
4. 渲染该 Shader 负责的所有 Mesh
5. 切换到下一个 Shader

为了做到这一点，必须引入Renderer类
## Renderer.h
首先需要修改前面章节的一个逻辑，在前一章的代码存档的主程序循环中，我先相乘了摄像机矩阵(viewProjectionMatrix)和模型矩阵(modelMatrix)得到最后的变换矩阵，然后将其传入着色器：
```cpp
// 处理shader的全局变量
glm::mat4 transMatrix = camera->getViewProjectionMatrix() * cube->getModelMatrix();
shader->setUniform4fv("aTransMatrix", transMatrix);
```
这样导致一些渲染逻辑不好解耦，我们还是考虑将这两个矩阵分别传入着色器：
```cpp
// Basic.shader
...
uniform mat4 modelMatrix;
uniform mat4 viewProjectionMatrix;

void main()
{
    gl_Position = viewProjectionMatrix * modelMatrix * vec4(aPos, 1.0);
    ...
}
```

然后假设：一个摄像机对应多个着色器，一个着色器对应多个物体，后面有例外了再来修改

```cpp
// Renderer.h
class Renderer {
public:
    static Renderer& getInstance(); // 获取单例实例

    void set_mesh_shader(Mesh* mesh, Shader* shader); // 更新 Mesh 所用的 Shader
    void Renderer::set_shader_camera(Shader* shader, Camera* camera); // 更新Shader所用Camera


    void renderAll(); // 统一渲染所有物体

private:
    Renderer() = default;
    ~Renderer() = default;

    std::unordered_map<Shader*, std::vector<Mesh*>> shaderMeshMap; // Shader → Mesh 组
    std::unordered_map<Shader*, Camera*> shaderCameraMap; // Shader → Camera
};
```
Renderer类同样是一个单例，在创建Mesh类和Shader类时，会调用set_mesh_shader和set_shader_camera函数，来存储对应关系，然后在renderAll()函数中，我们按Shader分组，处理全局变量并渲染物体:
```cpp
// Renderer.cpp
// 统一渲染所有物体
void Renderer::renderAll() {
    for (auto& [shader, meshes] : shaderMeshMap) {
        if (!shader) continue;
        shader->bind();

        // 获取 Shader 关联的 Camera，并设置变换矩阵
        Camera* camera = shaderCameraMap[shader];
        camera->set_shader_viewProjectionMatrix(shader);

        for (auto mesh : meshes) {
            mesh->render();
        }
    }
}
```
可以看到，由于已经正确按Shader分组，我们可以将全局变量设置放心移动到Camera类和Mesh类的渲染函数中，并且无需再处理Shader的绑定问题

## Light.h
在这里结束的话这一章有点太短了，顺便再添加一个Light类，搭建一个基础的光照测试场景

灯光也是一个可以被渲染的物体，所以它可以继承自Mesh类，并额外维护颜色、强度等信息。在实例化一个Light时，会默认创建一个小一些的立方体，用于标识其位置，并使用一个专属的着色器来渲染。同时先简单的用Renderer类管理创建的灯光，主着色器也添加了颜色项，灯光颜色会直接和采样的纹理相乘。代码中关于灯光的部分还有很多瑕疵，后面我们会慢慢修改


## 构建
在添加了Renderer类后，我们不需要在主函数中进行任何手动的绑定操作了，只需要创建所有的物体，并调用renderAll()，看上去非常好。构建此章节的代码存档，你可以看到灯光和被染上同样颜色的立方体：

<img src="assets\C4_0.png" style="zoom:50%;" />

