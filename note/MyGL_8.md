# 次序无关透明度、HDRI天空盒
这一章会涉及比较多的内容，首先实现LearnOpenGL中‘高级OpenGL/混合’章节中提到的次序无关透明度（Order Independent Transparency, OIT），然后加载比直接的立方体贴图更常用的HDRI天空盒贴图

## 准备
### FBO
首先我们需要一个前置技术，也就是OpenGL 帧缓冲对象（FBO），具体的细节这里跳过，LearnOpenGL中有对应的章节。和前面一样，在BufferObject.h中添加了FrameBuffer类，实现方法和前面的类也大同小异，主要是看在后面我们如何使用它

### 优化Renderer类
在本章当中，会添加4个新的shader，和一些绘制依赖，章节后的一帧渲染当中，需要经历天空盒、不透明物体、透明物体和灯光这4个不同的绘制过程，如果还是简单的向renderAll()函数中添加这些逻辑，那程序会变得非常难以维护，所以有必要对Renderer类做一个升级

首先我将大部分用new创建的对象都替换为了智能指针，例如所有的着色器、模型、灯光等，然后使用Renderer类统一创建管理这些资源，智能指针让我们无需担心忘记对象的析构，我在之前的代码存档中应该有犯这样的错误，因为在开发的前期，使用new创建对象还是更加方便，不过随着引入Renderer这样的资源管理类，后续要尽量避免使用new创建对象

然后是RenderPass基类，目前简单定义了一个execute()函数，我们拆分所有的渲染过程，在对应Pass中实现具体的渲染逻辑。然后在Renderer::setupRenderPasses()中注册它们

## OIT
OIT的实现方法有很多种，这里先实现比较简单的一种，Weighted Blended OIT即加权混合OIT，只需要使用帧缓冲就能完成

$
C_{\text{final}} = \frac{\sum (C_i \cdot \alpha_i \cdot w_i)}{\sum (\alpha_i \cdot w_i)}
$

其中：
- $C_i$是每个片段的颜色
- $\alpha_i$ 是片段的透明度
- $w_i$ 是一个权重函数，用于调整贡献度（通常选用 $\max(\alpha_i, 0.01)$避免零权重问题）

基本上就是累积片段的颜色，然后使用透明度做一个加权平均

具体实现的话，先使用OIT_accum.shader，向帧缓存对象中渲染所有的透明物体，记录累积的颜色和透明度，这里在片段着色器中：
```cpp
#shader fragment
#version 460 core
layout (location = 0) out vec4 accumColor;
layout (location = 1) out float accumAlpha;
```
定义两个通道的输出，我们就需要向帧缓冲对象中绑定两个颜色附件

然后使用OIT_draw.shader，向一个屏幕四边形上渲染帧缓冲对象中的纹理附件，将累积的颜色除以累积的透明度，以计算最终颜色

最终完整的渲染流程如下：
```cpp
void TransparentPass::execute()
{
    // 如果分辨率改变，则重新创建帧缓冲
    resizeIfNeeded(Config::screenWidth, Config::screenHeight);

    // 计算透明物体的颜色和透明度累积值
    glEnable(GL_BLEND);  // 启用混合
    glBlendFunc(GL_ONE, GL_ONE);  // 计算累积值，所以混合改为简单的相加模式
    oitFramebuffer->bind(); 
    glClear(GL_COLOR_BUFFER_BIT);
    glDepthMask(GL_FALSE);  // 禁用深度测试
    accumShader->bind();
    for(auto model : models){
        model->draw(accumShader.get());
    }
    glDepthMask(GL_TRUE);
    oitFramebuffer->unbind();

    // 接着将上面的累积结果绘制到屏幕四边形当中
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // 改回正常 Alpha 混合
    drawShader->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, oitFramebuffer->get_texture(0));
    drawShader->setUniform1i("accum_texture", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, oitFramebuffer->get_texture(1));
    drawShader->setUniform1i("alpha_texture", 1);
    dummy_screen->draw(drawShader.get());   // 利用屏幕四边形绘制结果

    glDisable(GL_BLEND);   // 关闭混合

}
// 创建帧缓冲，用于积累透明物体的颜色和透明度，添加两个颜色附件
void TransparentPass::resizeIfNeeded(int screenWidth, int screenHeight) {
    if (screenWidth != currentWidth || screenHeight != currentHeight) {
        std::vector<Framebuffer::AttachmentConfig> attachments = {
            {GL_RGBA, GL_RGBA16F, GL_FLOAT},
            {GL_RED, GL_R16F, GL_FLOAT}
        };
        oitFramebuffer = std::make_unique<Framebuffer>(screenWidth, screenHeight, attachments, false, false);
        currentWidth = screenWidth;
        currentHeight = screenHeight;
    }
}
```


## HDRI天空盒
直接使用立方体贴图作为天空盒现在并不常用，一般是使用HDRI天空盒贴图，这部分的理论知识在LearnOpenGL中的‘PBR/IBL/漫反射辐照’章节的开头部分，主要的改变都在Texture类当中。要加载一张.hdr后缀的图片作为天空盒，首先要用`Texture::load_hdr_texture()`中略微不同的方式创建纹理:
```cpp
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);  // 使用GL_RGB16F获得更大的颜色映射范围
```
此时的纹理是这样的：
<img src="assets\C8_0.png" style="zoom:50%;" />

也就是将周围的环境以等距柱状投影图(Equirectangular Map) 的方式存储在2D的图像中，我们直接对其按照等距柱状投影的规则来采样也是可以的，但将其先转换为立方体贴图，再采样会比较方便。转换的方式也很直观，就是将摄像机朝6个方向渲染，将渲染的画面存进立方体贴图即可

需要创建一个着色器`equirectangular_to_cubemap.shader`来按照等距柱状投影的规则采样hdr贴图，转换的代码在`Texture::convert_HDRI_to_cubemap()`中，我们首先创建一个空的立方体贴图，然后创建一个FBO用于离屏渲染，在6次循环中，切换摄像机的朝向，然后往立方体贴图对应的面写入纹理

在Renderer类中，添加绘制天空盒相关的变量：
```cpp
std::shared_ptr<Shader> skyBoxShader;
// 天空盒
std::shared_ptr<Texture> skyBoxCubemap;
std::shared_ptr<Mesh> dummyCube;
```
初始化它们：
```cpp
void Renderer::initialize()
{
    skyBoxShader = std::make_shared<Shader>("res/shader/Skybox.shader");
    skyBoxCubemap = std::make_shared<Texture>();
    skyBoxCubemap->add_hdri_to_cubemap("res/HDRI.hdr", 1024);
}
```
最后在渲染循环中调用：
```cpp
void SkyboxPass::execute()
{
    glDepthMask(GL_FALSE);
    shader->bind();
    texture->bind(shader.get());  // 传入 Shader 指针
    mesh->draw();
    glDepthMask(GL_TRUE);
}
```

## 构建
这一章的代码改动比较大，构建代码存档，可以看到，天空盒和透明物体被渲染，我没有按顺序绘制物体，但不管从前面还是后面观察，窗户的渲染都是对的。目前透明物体的渲染还比较简陋，例如透明度的效果比较不真实，而且也没有被光照影响，这些就先留到以后解决把

<img src="assets\C8_1.png" style="zoom:50%;" />
<img src="assets\C8_2.png" style="zoom:50%;" />


