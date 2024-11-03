# 简介

用于学习的渲染器

整个项目从学习OpenGL开始，网络上已经有非常多优质的教程，推荐一些：

[TheCherno] OpenGL, https://www.bilibili.com/video/BV1Ni4y1o7Au/?spm_id_from=333.788.videopod.episodes&vd_source=fe7a9ee6657422d709d30bf6284f347f&p=2

[LearnOpenGL CN], https://learnopengl-cn.github.io/

## 配置环境和项目搭建

使用VScode作为主要的代码编辑器，安装VS可以不安装IDE，只安装构建工具即可

安装工具：

https://visualstudio.microsoft.com/

https://code.visualstudio.com/

https://cmake.org/

安装方法省略，注意添加下面的环境变量即可：
```
D:\CMake\bin
D:\Visual Studio\2022\Community\VC\Tools\MSVC\14.41.34120\bin\Hostx64\x64
D:\Visual Studio\2022\Community\VC\Auxiliary\Build
```



创建项目文件夹：

### 编译glfw库

http://www.glfw.org/download.html

vender/文件夹用于存放第三方库的源代码，方便管理

在 .vscode/文件夹下创建tasks.json文件，开始编译我们需要的第一个库glfw：

```json
// tasks.json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "cmake build",
            "type": "shell",
            "command": "cmake",
            "args": [
                "--build",
                "${workspaceFolder}\\vender\\glfw-3.4\\build", // 指向构建目录
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
            "label": "cmake configure",
            "type": "shell",
            "command": "cmake",
            "args": [
                "${workspaceFolder}\\vender\\glfw-3.4",
                "-B",
                "${workspaceFolder}\\vender\\glfw-3.4\\build", // 指定构建目录
                "-G",
                "Visual Studio 17 2022" // 指定生成器
            ],
            "group": "none"
        }
    ]
}
```

先执行**`cmake configure` 任务**：

- 这个任务用于配置 CMake 项目。它会在 `./vender/glfw-3.4/build` 目录下生成所需的构建文件。
- `-G "Visual Studio 17 2022"` 指定使用 Visual Studio 2022 作为生成器。

再执行**`cmake build` 任务**：

- 这个任务用于构建项目。它会在之前配置的构建目录中使用 CMake 生成的构建文件进行编译。

### 配置vscode

**1**

打开命令面板（`Ctrl + Shift + P`），输入并选择 `C/C++: Edit Configurations (json)`，会在 `.vscode` 文件夹中创建 `c_cpp_properties.json`，编辑：

```json
//c_cpp_properties.json
{
    "configurations": [
        {
            "name": "Win64", // 根据需要调整
            "includePath": [
                "${workspaceFolder}/include",
                "${workspaceFolder}/vender/**"
                //"${workspaceFolder}/**" // 可选：用于递归查找
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE"
            ],
            "compilerPath": "D:/Visual Studio/2022/Community/VC/Tools/MSVC/14.41.34120/bin/Hostx64/x64/cl.exe",
            "cStandard": "c11", // 或 "c99"，根据需要
            "cppStandard": "c++17", // 根据需要
            "intelliSenseMode": "${default}" // 根据实际情况调整
        }
    ],
    "version": 4
}
```

这样vscode的智能提示功能就能正常工作了，后续有新的库的包含路径，需要在这里更新

**2**

我们还需要设置 Visual Studio 的开发环境，包括必要的环境变量（如 `INCLUDE` 和 `LIB`）。VS提供了一个批处理文件，可以确保编译器能够找到标准库和其他依赖项，在tasks.json中添加：

```json
"windows": {
        "options": {
            "shell": {
            "executable": "cmd.exe",
            "args": [
                "/C", // 运行结束后关闭终端
                "vcvarsall.bat“, 
                "x64",
                "&&" // 使用 && 确保在执行 vcvarsall.bat 后，接下来的命令会在同一个命令上下文中运行，这样设置的环境变量会影响到后续的 cl.exe 调用
            ]
            }
        }
     },
//...
```

**3**

接着添加一个task，用于编译整个项目：

```json
//tasks.json
//...
		{
            "label": "build_MyGL",
            "type": "shell",
            "command": "cl.exe",
            "args": [
                "/EHsc",
                "/Fe:${workspaceFolder}\\build\\MyGL.exe", // 输出文件路径
                "/Fo:${workspaceFolder}\\build\\",  // 编译中间文件路径
                "/I${workspaceFolder}\\include",
                "/I${workspaceFolder}\\vender\\glfw-3.4\\include",
                "${workspaceFolder}\\src\\*.cpp", // 源文件
                "/MDd", // 使用动态链接库
                "/link", // 链接库
                "${workspaceFolder}\\vender\\glfw-3.4\\build\\src\\Debug\\glfw3.lib", 
                // 必须的Window SDK库
                "opengl32.lib",
                "gdi32.lib",
                "user32.lib",
                "shell32.lib",
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": ["$msCompile"]
        }
```

后续有新的库的包含路径，源文件和lib文件，也需要更新这里

### 安装glad

https://glad.dav1d.de/

<img src="D:\_WORKSPACE\Note\assets\image-20241101210224056.png" alt="image-20241101210224056" style="zoom:50%;" />

将glad文件夹解压至vender/，修改tasks.json，添加包含目录和源文件glad.c的路径

这里有必要解释一下GLAD的用处，因为OpenGL只是一个标准/规范，具体的实现是由驱动开发商针对特定显卡实现的。由于OpenGL驱动版本众多，它大多数函数的位置都无法在编译时确定下来，需要在运行时查询。所以任务就落在了开发者身上，开发者需要在运行时获取函数地址并将其保存在一个函数指针中供以后使用，下面是在windows中手动获取OpenGL函数的步骤：

```c++
#include <windows.h>
#include <GL/gl.h>

typedef void (*PFNGLGENBUFFERSPROC)(GLsizei, GLuint*);
typedef void (*PFNGLBINDBUFFERPROC)(GLenum, GLuint);

int main() {
    // 创建 OpenGL 上下文的步骤（略）

    // 获取函数指针
    PFNGLGENBUFFERSPROC glGenBuffers = (PFNGLGENBUFFERSPROC) wglGetProcAddress("glGenBuffers");
    PFNGLBINDBUFFERPROC glBindBuffer = (PFNGLBINDBUFFERPROC) wglGetProcAddress("glBindBuffer");

    // 检查函数指针
    if (!glGenBuffers || !glBindBuffer) {
        std::cout << "Failed to load OpenGL functions" << std::endl;
        return -1;
    }

    // 使用 OpenGL 函数
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    // 其他 OpenGL 操作

    return 0;
}

```

### 创建窗口

以下示例为创建一个基本的窗口和OpenGL上下文

```c++
#include <GLFW/glfw3.h>
#include <iostream>

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    // 设置一些参数
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
```

### 画一个三角形



