# 物体描边
## UBO
因为后续会引入越来越多的shader，所以摄像机矩阵之类的不能再一个个设置了，使用UBO来管理这种着色器全局变量非常合适

之前介绍了SSBO，虽然SSBO在功能上可以包含UBO，但UBO还是更适合
传递小规模、全局一致性的数据，比如：变换矩阵、材质参数等，UBO 在这些场景下通常比 SSBO 更快，因为 UBO 是专门为快速访问设计的

UBO的使用和之前的缓冲对象没有太大的区别，我在BufferObject中添加了UBO类，使用也非常简单，在Camera类中添加一个UBO成员，并在构造函数中：
```cpp
cameraUBO = new UBO (sizeof(glm::mat4)+sizeof(glm::vec3), 0); // 创建 UBO（存储一个 mat4和一个vec3）, 绑定点 0
cameraUBO->PushData(viewProjectionMatrix);
cameraUBO->PushData(pos);
```
对应的，着色器中的相关全局变量也改为UBO
```cpp
layout(std140, binding = 0) uniform CameraUBO {
    mat4 viewProjectionMatrix;
    vec3 uViewPos;
};
```
然后在摄像机中更新即可：
```cpp
void Camera::updataViewProjectionMatrix()
{
    viewProjectionMatrix = projectionMatrix * viewMatrix;
    cameraUBO->UpdateData(&viewProjectionMatrix, sizeof(glm::mat4), 0);
}
```

*第7，8节的代码存档中，着色器中的viewPos实际上没有实时更新，这个bug在第9节中被修复了*

## 模板测试
为了绘制轮廓，需要一个单色着色器，我选择在Model类中添加一个静态Shader实例，来加载这个着色器，注意在这种情况下，需要使用**懒加载**，保证这个静态变量在创建第一个模型类实例时才初始化，否则静态变量会在OpenGL上下文创建之前被加载，从而报错

然后为Model类添加一个`draw_outline()`方法，用于绘制物体描边，具体的方法和LearnOpenGL中一致

## 构建
构建本节的代码存档，可以看到物体的描边已经被添加：

<img src="assets\C7_0.png" style="zoom:50%;" />