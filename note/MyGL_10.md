# 延迟渲染
将目前的Basic.shader初步迁移至延迟渲染管线其实非常简单，只需要将其拆分为两个着色器，Deferred_G.shader和Deferred_L.shader，分别用于G-Pass和光照Pass。在Deferred_G.shader的片段着色器中：
```cpp
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


    // 存储第一个G缓冲纹理中的片段位置向量
    gPosition = fs_in.FragPos;
    // 存储第二个G缓冲纹理中的片段法线向量(世界空间)
    gNormal = worldNormal;
    // 和漫反射对每个逐片段颜色
    gAlbedoSpec.rgb = texture(texture_diffuse0, shiftTexCoord).rgb;
    // 存储镜面强度到gAlbedoSpec的alpha分量
    gAlbedoSpec.a = texture(texture_specular0, shiftTexCoord).r;
}
```
直接应用了使用高度贴图偏移后的纹理坐标值和世界坐标法线，这样法线贴图和高度贴图就无缝兼容了延迟渲染。然后将光照相关的所有内容复制到Deferred_L.shader中，将对应的变量从G-buffer中采样即可。然后新增一个`OpaqueDeferredPass`替换原来的不透明物体渲染，非常简单

## 一些问题
### G-buffer默认值
当用 glTexImage2D(..., NULL) 创建纹理并附加到帧缓冲时，OpenGL 并不会自动清空它们的内容，而是保留不确定值。这里我们可以在G-buffer pass前执行：
```cpp
glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
```
将纹理内容清空，这意味着在G-buffer pass中没有绘制任何几何体的片段就会是这个默认值，这个默认值对于纹理中存储的内容（如片段位置、法线等）很多时候意义并不明确，可能会导致渲染错误，这一点要注意。当引入深度剔除后一般影响不大。除此之外，上面的清空方式其实并不适用于多颜色附件，这只能清除第一个 GL_COLOR_ATTACHMENT0，我们应该指定每个附件的清除：
```cpp
glClearBufferfv(GL_COLOR, 0, glm::value_ptr(glm::vec3(0.0f))); // gPosition
glClearBufferfv(GL_COLOR, 1, glm::value_ptr(glm::vec3(0.0f))); // gNormal
glClearBufferfv(GL_COLOR, 2, glm::value_ptr(glm::vec4(0.0f))); // gAlbedoSpec
glClear(GL_DEPTH_BUFFER_BIT);
```
### 天空盒被遮挡
其实很容易想到，由于延迟渲染的光照Pass会在一个屏幕四边形上渲染，所有就算实际上没有几何体，应该透出天空的部分，也会被渲染并遮挡住天空盒。这里的解决方法有很多，一种是选择将深度也存入G-buffer，将gPosition修改为4通道，在.a通道存入片段深度。然后关键是：
```cpp
deferredFramebuffer->bind(); // 绑定帧缓冲
glClearBufferfv(GL_COLOR, 0, glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f))); // gPosition
```
将其默认值刷新为1，然后在光照Pass中，直接丢弃深度为1的片段即可。这也是为什么上面要提到纹理默认值的问题

这样好处是不用修改天空盒的渲染了，而且深度G-buffer迟早要用到。但想了想，后面还有灯光绘制这个正向步骤，还是统一将这些正向渲染用交换深度附件的方式做深度测试更一致，为此，需要将FrameBuffer统一在Renderer中创建，然后为FrameBuffer类添加`switch_depth_component_to_default`方法，然后渲染天空盒和灯光立方体的时候就可以：
```cpp
void SkyboxPass::execute()
{
    if(ifDeferred){
        Renderer::getInstance().get_deferredFramebuffer()->switch_depth_component_to_default(); // 将深度缓冲切换到默认帧缓冲
    }
    // glDepthMask(GL_FALSE);
    shader->bind();
    texture->bind(shader.get());  // 传入 Shader 指针
    dummyCube->draw();
    // glDepthMask(GL_TRUE);
}
```
我们启用天空盒渲染时的深度测试功能，然后对SkyBox.shader的片段着色器做一点修改：
```cpp
// 手动设置深度值
gl_FragDepth = 0.9999; // 设置为远平面深度
```
这样保证天空盒在正常物体的后面，同时这里不能是1，不然其本身过不了深度测试

### 深度G-buffer的必要性
到目前为止你可能觉得在G-buffer中存入深度没有什么必要，因为在光照Pass中我们有FragPos和ViewPos，完全可以计算出片段的深度，而且上面天空盒被遮挡问题也完全有别的解决方法，并且当结合延迟渲染和正向渲染时，深度测试的问题也有两种解决方法：
1. 在G-buffer pass中将深度信息同样存入纹理，然后在正向Pass中手动处理片段的丢弃
2. 将G-buffer pass中的深度附件切换至正向pass的帧缓冲中，自动执行深度测试

| 对比项 | 使用深度纹理手动判断（方式 1） | 使用深度缓冲做硬件测试（方式 2） |
|--------|----------------------------------|-----------------------------------|
| 实现灵活性 | ✅ 高，可自定义处理逻辑            | ❌ 限制较大，仅靠硬件处理        |
| 兼容屏幕空间特效 | ✅ 非常好（SSAO、雾、反射）      | ❌ 需要额外采样处理或 pass       |
| 性能（单纯光照） | ❌ 多一次纹理采样                 | ✅ GPU 原生深度测试更快         |
| 内存占用 | ❌ 多一张深度纹理                  | ✅ 只用一个 RBO/Texture         |
| 可维护性 | ✅ 清晰易 debug                    | ✅ 更贴近传统 OpenGL 管线       |

---

看上去完全可以选择不将深度信息存入G-buffer中，但其实深度纹理还是有非常大的作用，例如各种屏幕空间效果（SSAO、景深效果等），高级的光体积剔除，甚至可以用深度反推片段位置，反倒可以节省下gPosition的空间


## 构建
本节主要是切换至了延迟渲染，渲染效果本身没有什么变化，直接查看下一节的存档即可
