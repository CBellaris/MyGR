# 基础光照
这一章会实现一个简单的风氏光照模型(Phong Lighting Model)，原理部分没有太多可以讲的，我的着色器代码和LearnOpenGL中光照这一章节展示的着色器代码相似，添加了对环境光照项(ambient)的限制和移除平行光的反射项。最主要的部分是使用SSBO(Shader Storage Buffer Object)处理灯光：
```cpp
// Basic.shader
// ...
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
// ...
```
这让我们可以方便的管理不同类型的灯光，并在运行时动态增添和修改光源，其实就目前实现的功能来看，使用SSBO有点杀鸡用牛刀了，不过提前引入这个功能也不错

## SSBO
Shader Storage Buffer Object（SSBO）是 OpenGL 4.3+ 引入的特性，主要用于实现 CPU 与 GPU 之间的大规模数据交互，例如大规模结构化数据传输（如粒子系统、骨骼动画）

### 创建
在BufferObject.h中，我新增了SSBO类，封装了一些基础的功能。首先，创建一个SSBO和其他的缓冲对象是一样的：
```cpp
// BufferObject.h
template <typename T>
class SSBO {
public:
    SSBO(GLuint bindingPoint, unsigned int maxDataSize) : bindingPoint(bindingPoint) {
        bufferSize = maxDataSize * sizeof(T);
        glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
    // ...
};
```
与最开始的VBO等不同的是，需要额外处理**绑定点**(binding point)，所有的着色器程序都可以通过访问此绑定点获取数据。这里的glBufferData可以用于写入数据，SSBO被创建后依旧可以使用其改变大小和写入其他数据，但频繁改变缓冲区的大小并不好，所以我在这里选择在创建时指定一次想要使用的内存大小，后续只用glBufferSubData来写入数据：
```cpp
// BufferObject.h
template <typename T>
void SSBO<T>::updateData(const std::vector<T>& data, size_t offset) {
    size_t dataSize = data.size() * sizeof(T);
    if (offset + dataSize > bufferSize) {
        throw std::runtime_error("SSBO::updateData: Data exceeds buffer size");
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, dataSize, data.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
```

### 数据结构对齐：C++ 与 GLSL 的映射
需要在C++代码端和GLSL中定义完全一致的两个结构体，才可以正确的传递数据。除此之外，还需要特别注意内存的对齐，以这一章着色器中的灯光单元为例：
```cpp
// Light.h
struct LightUnit {
    alignas(16) glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);  
    alignas(16) glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    alignas(16) glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);     
    alignas(16) glm::vec3 intensity = glm::vec3(0.2f, 0.5f, 0.5f);  // 分量分别为ambient, diffuse和specular的强度

    alignas(4) float constant = 1.0f;
    alignas(4) float linear = 	0.14f;
    alignas(4) float quadratic = 0.07f;

    // 默认可见并且为点光
    alignas(4) int visibility = 1;
    alignas(4) int isDirectional = 0;

    alignas(4) float padding1;
    alignas(4) float padding2;
    alignas(4) float padding3;
};
// -------------------------------------------------
// Basic.shader
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
```
1. 不管是vec3还是vec4，在着色器中会统一填充至16字节
2. 避免使用bool类型，因为在着色器中会被填充至4字节，不太好管理
3. 标量类型（float, int）4 字节对齐
4. 整个的结构体需要填充至16字节的倍数

根据上面的原则，就可以看懂这里为什么需要这样定义结构体


## 构建
以管理多光源为契机，提前学习了SSBO，如果你还想更深入的了解，可以阅读LearnOpenGL的高级OpenGL/高级GLSL章节，其中的UBO和SSBO相似，但SSBO是更现代的特性

构建本章的代码存档，可以看到被多光源照亮的金属立方体，简单调用Light类的接口，就可以实时修改光源的属性，动态调整灯光数量：

<img src="assets\C5_0.png" style="zoom:50%;" />