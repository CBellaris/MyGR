# 简介

用于学习的3D渲染器

教程的前半部分，可以看做是对于OpenGL经典教程：[LearnOpenGL CN], https://learnopengl-cn.github.io/ 的实现和补充，不会再次提及教程中已经有的关于原理的内容，而会从编写一个渲染器的项目角度，梳理整个流程，补充一些我在开发过程中遇到的一些问题，包括但不限于：添加UI，
后续：添加光线追踪
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

<img src="assets\image-20241101210224056.png" alt="image-20241101210224056" style="zoom:50%;" />

将glad文件夹解压至vender/，修改tasks.json，添加包含目录和源文件glad.c的路径

简单解释一下GLAD的用处，因为OpenGL只是一个标准/规范，具体的实现是由驱动开发商针对特定显卡实现的。由于OpenGL驱动版本众多，它大多数函数的位置都无法在编译时确定下来，需要在运行时查询。所以任务就落在了开发者身上，开发者需要在运行时获取函数地址并将其保存在一个函数指针中供以后使用，下面是在windows中手动获取OpenGL函数的步骤：

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

## 创建窗口

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

## 画一个正方形
下面是一个用于绘制简单正方形的示例程序，你可以用它来测试环境的搭建正不正确，其中包括创建VAO, VBO, IBO和shader，最后绑定它们并绘制两个三角形。再提醒一下，LearnOpenGL中已经讲解过的内容本教程不会再赘述，例如对于下面的代码，你可以先阅读LearnOpenGL中的"入门/你好，三角形"章节
 ```c++
 // main.cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

static void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// 回调函数，当窗口大小改变时，GLFW会调用这个函数，我们在这里同步改变OpenGL的视口大小
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// 下面三个函数用于处理shader
// 解析shader中的源码，分割为两个字符串，分别为vertex和fragment shader
static std::vector<std::stringstream> ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    ShaderType type = ShaderType::NONE;
    std::string line;
    std::vector<std::stringstream> ss(2);
    while(getline(stream, line))
    {
        if(line.find("#shader") != std::string::npos)
        {
            if(line.find("vertex") != std::string::npos)
            {
                type = ShaderType::VERTEX;
            }
            else if(line.find("fragment") != std::string::npos)
            {
                type = ShaderType::FRAGMENT;
            }
        }
        else
        {
            ss[static_cast<int>(type)] << line << '\n';
        }
    }

    return ss;
}

// 接受一段shader源码，将其编译
static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    // 获取异常
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if(result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = new char[length];
        glGetShaderInfoLog(id, length, &length, message);
        std::cout<<"Failed to compile the "<<(type == GL_VERTEX_SHADER ? "VertexShader:" : "FragmentShader:")<<message<<std::endl;
        delete[] message;
        glDeleteShader(id);
        return 0;
    }
    
    return id;
} 

// 编译两个shader，并链接为一个程序
static unsigned int CreateShader(const std::string& VertexShader, const std::string& FragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, VertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, FragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);
    int isLinked;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) 
    {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char* message = new char[length];
        glGetProgramInfoLog(program, length, &length, message);
        std::cout << "Failed to link program: " << message << std::endl;
        delete[] message;
        return 0;
    }


    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

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
    window = glfwCreateWindow(800, 600, "Hello World", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // 加载glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 定义视口大小
    glViewport(0, 0, 800, 600);


    // 注册回调函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 定义顶点数据
    float vertices[] = {
    -0.5f, -0.5f,
     0.5f, -0.5f,
     0.5f,  0.5f,
     -0.5f,  0.5f
    };

    // 定义索引数组
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };


    // 创建顶点数组对象(Vertex Array Object, VAO)
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    // 绑定VAO以记录VBO，IBO和数据layout
    glBindVertexArray(VAO);

    // 创建顶点缓冲对象(Vertex Buffer Objects, VBO)
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  
    // 把用户定义的数据复制到当前绑定缓冲, GL_STATIC_DRAW ：数据不会或几乎不会改变; GL_DYNAMIC_DRAW：数据会被改变很多; GL_STREAM_DRAW ：数据每次绘制时都会改变
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 指定属性的格式(第几个属性，包含几个数据，数据类型，是否标准化，一个顶点的步长，到下一个属性的指针)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, (const void*)0);
    // 启用
    glEnableVertexAttribArray(0);

    // 创建索引缓冲对象(Index Buffer Object，IBO)
    unsigned int IBO;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 解绑VAO，使用时再绑定
    glBindVertexArray(0);

    // 读取shader源码
    auto shadersCode = ParseShader("res/shader/Basic.shader");
    std::string vertexShaderCode = shadersCode[0].str();
    std::string fragmentShaderCode = shadersCode[1].str();
    // 创建shader
    unsigned int shader = CreateShader(vertexShaderCode, fragmentShaderCode);

    //绘制前绑定VAO和shader
    glUseProgram(shader);
    glBindVertexArray(VAO);
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        processInput(window);
        // 检查并调用事件，交换缓冲
        glfwPollEvents();
        glfwSwapBuffers(window);
        
    }
    glDeleteProgram(shader);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);


    glfwTerminate();
    return 0;
}
 ```

Basic.shader单独存放在res/shader，注意需要将其复制到build/res/shader，运行程序时才能正常读取到：
```
#shader vertex
#version 460 core
layout (location = 0) in vec2 aPos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0f, 1.0f);
}


#shader fragment
#version 460 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
} 
```

