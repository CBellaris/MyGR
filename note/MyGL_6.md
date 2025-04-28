# 导入模型

## 安装assimp
在vender文件夹下运行(如果你保持和我一样的目录结构)：
```bash
git clone https://github.com/assimp/assimp.git
```
然后向tasks.json添加两个构建任务用于编译这个库：
```json
{
            "label": "cmake build assimp",
            "type": "shell",
            "command": "cmake",
            "args": [
                "--build",
                "${workspaceFolder}\\vender\\assimp\\build", // 指向构建目录
                "--config",
                "Debug" // 或 "Release"，根据需要
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "cmake configure assimp",
            "type": "shell",
            "command": "cmake",
            "args": [
                "${workspaceFolder}\\vender\\assimp",
                "-B",
                "${workspaceFolder}\\vender\\assimp\\build", // 指定构建目录
                "-G",
                "Visual Studio 17 2022" // 指定生成器
            ],
            "group": "none"
        },
```
同样先运行configure，再运行build

然后修改tasks.json中的项目构建任务，需要添加两个include路径，还有库文件：
```json
// ...
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
                "${workspaceFolder}\\src\\*.cpp", // 源文件
                "${workspaceFolder}\\vender\\glad\\src\\glad.c", // 库源文件
                "${workspaceFolder}\\vender\\stb_image\\stb_image.cpp", // 库源文件
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

最后复制./vender/assimp/build/bin/Debug下的.dll和.pdb文件至./build

## Model.h
Model类代码与LearnOpenGL中的模型加载章节给出的代码示例基本一致，不同的是，我将与材质相关的功能都移动至了Texture类，Mesh类现在包含一个Texture类实例，可以管理多张纹理，并且自动处理不同纹理类型。并且支持纹理缓存功能，只需要在Texture类中维护一个静态的变量：`static std::unordered_map<std::string, unsigned int> textureCache;`，即可清晰的记录已经加载的纹理，避免重复加载。将纹理相关功能从Model类中解耦后，在Model类的processMesh函数中，只需要查询所有的纹理，并添加至texture实例中即可：
```cpp
for(unsigned int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++)
    {
        aiString str;
        material->GetTexture(aiTextureType_DIFFUSE, i, &str);

        std::string filepath = this->directory + '/' + str.C_Str();
        texture->add_image(filepath, TextureType::Diffuse);
    }
// ... 对其他的纹理类型重复相同的操作即可
```

我对其他类也做出了一些改变，例如Light类现在不继承自Mesh了，而是在内部单独维护一个mesh实例，并在Renderer类的renderAll()中单独绘制。Renderer类中记录的渲染关系也做了一些简化，默认只有一个摄像机，并将Shader → Mesh组替换为了Shader → Model组：```std::unordered_map<Shader*, std::vector<Model*>> shaderModelMap; ```等等。总而言之，我们在主函数中的调用变得更加简洁了，只需要定义好摄像机，灯光，着色器和模型，即可渲染场景

## 未来优化
目前一个模型类包含很多个Mesh实例，而每个Mesh都维护一个VBO，在渲染模型时，依次切换绑定每个Mesh的VBO并渲染的开销是比较大的，例如下面的背包模型，大概分为了30个网格。对于这样的静态模型，（即顶点数据和索引数据不会在运行时发生变化的模型），可以通过合批渲染来优化性能。合批渲染的主要思想是将多个 Mesh 的顶点数据和索引数据合并到一个大的 VBO 和 IBO 中，从而减少 OpenGL 的绘制调用次数

但是合批渲染的一个麻烦之处就是纹理的处理，当每个 Mesh 使用不同的纹理时，由于OpenGL 不支持在一次绘制调用中动态切换纹理，我们就无法在一次渲染调用下正确绘制。需要使用纹理图集和多纹理绑定等来解决，这样工作量就非常大了，我们的简单渲染器应该不会在短时间内遇到性能瓶颈，就留到以后解决了

## 构建
（本节的代码存档忘记存了，可以直接构建下一节的代码存档，区别不大）res中添加了LearnOpenGL中的同款背包模型，并使用新添的Model类加载它：```Model* model = new Model("res/backpack/backpack.obj");```，可以看到，渲染的效果还是非常棒的

<img src="assets\C6_0.png" style="zoom:50%;" />

<img src="assets\C6_1.png" style="zoom:50%;" />