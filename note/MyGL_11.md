# SSAO
在添加SSAO之前，我们可以先尝试读取一些更复杂的场景，可以更清晰的看到SSAO带来的效果，为此我们的渲染器需要一点升级

## 内嵌纹理
Assimp对一些格式的支持有限，例如.blend，基本上新版blender（4.0以后）保存的文件在读取时都会报错，所以将模型文件转换为.obj或者.fbx是比较常见的，在这个过程中还要带着所有的外部纹理，实在是很容易出错。一个比较好的选择是转换为.fbx文件，可以将纹理内嵌入模型文件，在blender中导出为.fbx时将路径模式选择为复制，并勾选后面的内嵌纹理：

<img src="assets\C11_0.png" style="zoom:50%;" />

然后需要修改模型的导入时的纹理处理方式，在`void Model::process_material_textures(...)`中：
```cpp
// Assimp 样式内嵌贴图路径 "*0"
if (!texPath.empty() && texPath[0] == '*') {
    int index = std::stoi(texPath.substr(1));
    if (index >= 0 && index < static_cast<in(scene->mNumTextures)) {
        embeddedTex = scene->mTextures[index];
    }

// FBX 虚拟路径，如 "scene.fbm/Tex_0001.png"
if (!embeddedTex) {
    for (unsigned int t = 0; t < scene->mNumTextures; ++t) {
        if (scene->mTextures[t]->mFilename.C_Str() =texPath) {
            embeddedTex = scene->mTextures[t];
            break;
        }
    }
}
}
```
如果是内嵌纹理，那么Assimp加载的纹理路径会有上面两种形式，一般.obj内嵌纹理会是上面的`"*0"`形式，这时可以通过后面的数字索引对应的纹理，然后如果是.fbx则会生成一个虚拟路径，通过这个虚拟路径索引到纹理

这时候还需要判断纹理是压缩类型(e.g., PNG/JPG) （即`if (embeddedTex->mHeight == 0)` ）还是非压缩类型，压缩类型依旧需要stb_image来解析（使用stbi_load_from_memory），非压缩类型则可以直接创建OpenGL纹理。注意对于实际用Texture类加载图像时，这个时候我们拿到的是数据的指针，纹理路径仅作标识符，所以`add_image()`要做出一些修改，并且新增了`add_image_from_raw()`方法

## ImGui
https://github.com/ocornut/imgui
C++最常用的GUI库，安装不需要额外编译，我依旧是将整个项目clone到./vender/assimp，现在的构建任务长这样：
```json
{
"label": "build_MyGL",
"type": "shell",
"command": "cl.exe",
"args": [
    "/std:c++17",
    "/EHsc",
    "/Zi", // 需要调试时启用
    //"/Od", // 禁用优化，确保断点准确
    "/source-charset:utf-8", // 修复字符集警告
    "/Fe:${workspaceFolder}\\build\\MyGR.exe", // 输出文件路径
    "/Fo:${workspaceFolder}\\build\\",  // 编译中间文件路径
    "/Fd:${workspaceFolder}\\build\\", // 指定.pdb文件路径
    "/I${workspaceFolder}\\include",
    "/I${workspaceFolder}\\vender",
    "/I${workspaceFolder}\\vender\\glfw-3.4\\include",
    "/I${workspaceFolder}\\vender\\glad\\include",
    "/I${workspaceFolder}\\vender\\stb_image",
    "/I${workspaceFolder}\\vender\\assimp\\include",
    "/I${workspaceFolder}\\vender\\assimp\\build\\include",
    "/I${workspaceFolder}\\vender\\json\\single_include",
    "/I${workspaceFolder}\\vender\\imgui",
    "/I${workspaceFolder}\\vender\\imgui\\backends",
    "${workspaceFolder}\\src\\*.cpp", // 源文件
    "${workspaceFolder}\\vender\\glad\\src\\glad.c", // glad库源文件
    "${workspaceFolder}\\vender\\stb_image\\stb_image.cpp", // stb_image库源文件
    "${workspaceFolder}\\vender\\imgui\\*.cpp", // imgui库源文件
    "${workspaceFolder}\\vender\\imgui\\backends\\imgui_impl_glfw.cpp", // imgui库源文件
    "${workspaceFolder}\\vender\\imgui\\backends\\imgui_impl_opengl3.cpp", // imgui库源文件
    "/MDd", // 使用动态链接库(uncheck reason)
    "/link", // 链接库
    "${workspaceFolder}\\vender\\glfw-3.4\\build\\src\\Debug\\glfw3.lib", 
    "${workspaceFolder}\\vender\\assimp\\build\\lib\\Debug\\assimp-vc143-mtd.lib", 
    // 必须的Window SDK库
    "gdi32.lib",
    "user32.lib",
    "shell32.lib",
],
"group": {
    "kind": "build",
    "isDefault": true
},
"dependsOn": "Copy res files", // 复制res文件到build
"problemMatcher": ["$msCompile"]
}
```
然后在main.cpp中添加ImGui：
```cpp
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// ...

// 创建建窗口并设置 GLAD 之后：
// 创建 ImGui 上下文
IMGUI_CHECKVERSION();
ImGui::CreateContext();
ImGuiIO& io = ImGui::GetIO(); (void)io;
io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 启用键盘控制（可选）
// 设置风格
ImGui::StyleColorsDark();
// 初始化 ImGui GLFW 和 OpenGL3 实现
ImGui_ImplGlfw_InitForOpenGL(window, true);
ImGui_ImplOpenGL3_Init("#version 460");

//...

// 主循环 Renderer::getInstance().renderAll(); 之后：
// 开始新一帧 ImGui
ImGui_ImplOpenGL3_NewFrame();
ImGui_ImplGlfw_NewFrame();
ImGui::NewFrame();
// 示例 UI
ImGui::Begin("调试面板");
ImGui::Text("Hello, ImGui!");
ImGui::End();
// 渲染 ImGui
ImGui::Render();
ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

// ...

//  glfwTerminate(); 之前：
ImGui_ImplOpenGL3_Shutdown();
ImGui_ImplGlfw_Shutdown();
ImGui::DestroyContext();
```

## SSAO
目前就简单的将SSAO实现在不透明物体的延迟渲染Pass当中，大概是需要下面几个改动：

1. 纹理类添加了对噪声贴图的支持：`void Texture::add_noise_texture()`

2. 不透明物体的延迟渲染Pass添加一个采样核生成函数：`std::vector<glm::vec3> OpaqueDeferredPass::generateSSAOKernel(int kernelSize)`

3. 新建一个用于SSAO的FrameBuffer和着色器SSAO.shader，同样在Renderer类中初始化和管理

4. 然后就是将SSAO采样阶段插入到`void OpaqueDeferredPass::execute()`当中，更新一下Deferred_L.shader，加入SSAO的采样结构

重点来说一下深度值的优化：

在LearnOpenGL的原文中，gPosition保存的fragPos是在视空间中的，也就是viewMatrix乘以aPos，这其实有点麻烦，因为后续的光照Pass需要的是世界空间的fragPos，那就需要单独为SSAO写入一个G-buffer，没必要。同时在线性深度空间比较深度比较麻烦，不仅需要额外传入远平面位置，而且当远平面值比较大时，依旧会有映射到[0,1]带来的精度问题

所以我选择在G-buffer阶段直接将片段实际和摄像机的距离写入纹理：
```cpp
// 实际片段深度
float linearDepth = length(fs_in.viewPos - fs_in.FragPos);
// 存储第一个G缓冲纹理中的片段位置向量
gPosition = vec4(fs_in.FragPos, linearDepth); // 将深度信息存储在gPosition.a中
```
然后是SSAO采样阶段，直接在世界坐标系中操作，计算采样点到摄像机的距离：
```cpp
// 获取样本位置
vec3 sample_unit = TBN * samples[i]; // 切线->世界空间
sample_unit = fragPos + normal * bias + sample_unit * radius; // 添加偏移防止自遮蔽
float sample_depth = length(sample_unit - fs_in.viewPos); // 计算实际采样点深度
```
然后得到实际片段的深度（和摄像机的距离）：
```cpp
// 获取实际片段深度
vec4 sampleClip = fs_in.projection * vec4(sample_unit, 1.0); // 世界->裁剪空间
sampleClip.xyz /= sampleClip.w; // 透视除法
sampleClip.xyz = sampleClip.xyz * 0.5 + 0.5; // 归一化到[0,1]
float sampleDepth = texture(gPosition, sampleClip.xy).a;
```
后续比较这两个值就可以。注意还可以将采样点沿着法线上移一个很小的偏移值，防止自遮蔽现象

## 加入UI控制
首先要可以释放我们的鼠标，在InputManager.h的keyCallback函数中：
```cpp
if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
    if(instance.mouseLocked) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // 释放鼠标
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // 锁定鼠标
    }
    instance.mouseLocked = !instance.mouseLocked; // 切换状态
}
```
加入一个mouseLocked变量，由tab键切换

在初始化ImGui时的这个操作：
```cpp
ImGui_ImplGlfw_InitForOpenGL(window, true);
```
实际上自动将OpenGL的各种回调函数注册给了ImGui，但如果我们后续使用自己的回调覆盖了OpenGL的，那么ImGui也会接收不到回调的信息。所以后续所有在InputManager中自定义的回调函数，都需要进行一个转发操作，类似：
```cpp
static void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos); // 转发给 ImGui

    // ...
}
```

关于传递变量给ImGui，现在先非常简单的向Renderer类中添加一个公共成员ssaoStrength做测试，然后新增ImGuiPass绘制UI

当然直接这样传递公共成员不是长久之计，后面我们会升级为反射系统，更高效的管理所有的属性

## 构建
加载scene.fbx，这是由一个.pmx模型转换而来：https://www.aplaybox.com/details/model/NJ3h079ACJkU

顺便写一下如何转换.pmx文件，在blender中安装这个插件：https://github.com/MMD-Blender/blender_mmd_tools 直接在偏好设置->拓展中搜索这个插件安装就可以，然后点击导入模型，重点是选中导入的模型，点一下最底下材质中的*转换给Blender*，然后按上文说的导出即可

<img src="assets\C11_1.png" style="zoom:50%;" />


调整好视角后，按tab键释放鼠标，就可以拖动滑条控制SSAO的强度，下图的对比中可以清晰的看到SSAO的效果，很不错，后续还可以加入模糊等升级

<img src="assets\C11_2.png" style="zoom:50%;" />

<img src="assets\C11_3.png" style="zoom:50%;" />


更多的场景渲染效果：

<img src="assets\C11_4.png" style="zoom:50%;" />

<img src="assets\C11_5.png" style="zoom:50%;" />