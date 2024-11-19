## 封装渲染流程
通过前面的步骤，我们已经搭建好了环境，并运行了一段简单的主程序绘制了一个正方形。为了创建一个渲染器，需要将与绘制相关的操作都用类封装起来，例如创建VAO, VBO, IBO这些缓冲对象，编译着色器等操作。接下来几个章节，我会基本按照LearnOpenGL教程的顺序，将渲染操作逐步封装进类中，并且提前引入实例化渲染（Instanced Rendering），Uniform Buffer Object (UBO) 等特性，这会对后续的高级效果开发带来便利

接下的内容仅提及大概思路，详细的代码存档可以在。。。查看
### BufferObject
为了管理一组顶点数据，一般的流程是：绑定 VAO；绑定 VBO，并将顶点数据传入；通过 glVertexAttribPointer 函数来定义顶点属性；绑定 IBO 并将索引数据传入；解绑 VAO；后续想要使用这组数据来渲染时，再绑定VAO并渲染。也就是说我们以VAO为单位管理数据即可，这里当然还可以进一步管理，例如一个VAO其实可以绑定多个VBO和一个IBO，绑定后还可以修改VBO和IBO的数据，但这会增加太多复杂性，我们先用简单点的思路来编写BufferObject类，后续再来拓展，即对于一个创建完成的VAO，不解绑VBO和IBO，如果你需要更改顶点数据的布局（例如，添加一个新的顶点属性或更改属性格式），直接创建新的VAO

BufferObject类可以这样使用：
```cpp
// 定义顶点数据
std::vector<float> vertices = {
    -0.5f,-0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, 1.0f, 0.0f,
    0.5f,  0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.0f, 1.0f
};
// 定义索引数组
std::vector<unsigned int> indices = {
    0, 1, 2,
    2, 3, 0
};

VertexArrayObject* VAO = new VertexArrayObject();
VAO->addVertexBuffer(vertices);
VAO->addIndexBuffer(indices);
VAO->push<float>(2);
VAO->push<float>(2);
VAO->bindAll();
// ...
// 使用时绑定
VAO->bind();

### Shader
先创建一个非常基础的Shader类，就是将上一节实例程序中读入、编译着色器的几个函数移动到类中，并提供接口来设置着色器程序中的全局变量，后续我们就能用几行代码来创建着色器程序：

```


## 错误处理
在早些的版本中，一般使用glGetError获取OpenGL函数的错误信息，在OpenGL 4.3或更新的版本中，提供了一种新的调试机制，`glDebugMessageCallback`
### 启用调试输出
在创建 OpenGL 上下文时，需要将其配置为调试上下文。在一些窗口系统（如 GLFW）中可以直接指定调试标志。

示例（GLFW）：
```cpp
glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
```
### 启用调试输出并注册回调函数
调用 glEnable 来启用调试输出，然后使用 glDebugMessageCallback 注册回调函数

```cpp
#include <GL/glew.h>
#include <iostream>

// 调试消息回调函数
void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                               GLsizei length, const GLchar* message, const void* userParam) {
    // 获取错误的源和类型信息
    std::string sourceStr;
    switch (source) {
        case GL_DEBUG_SOURCE_API: sourceStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: sourceStr = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION: sourceStr = "Application"; break;
        case GL_DEBUG_SOURCE_OTHER: sourceStr = "Other"; break;
    }

    std::string typeStr;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR: typeStr = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "Undefined Behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY: typeStr = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "Performance"; break;
        case GL_DEBUG_TYPE_OTHER: typeStr = "Other"; break;
    }

    std::string severityStr;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: severityStr = "High"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "Medium"; break;
        case GL_DEBUG_SEVERITY_LOW: severityStr = "Low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
    }

    // 打印出错误的详细信息
    std::cout << "OpenGL Debug Message: " << std::endl;
    std::cout << "  Source: " << sourceStr << std::endl;
    std::cout << "  Type: " << typeStr << std::endl;
    std::cout << "  Severity: " << severityStr << std::endl;
    std::cout << "  Message: " << message << std::endl;
}

int main() {
    // 初始化 OpenGL 环境，确保创建了调试上下文
    // ...

    // 启用调试输出
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);  // 确保调试信息立即输出

    // 注册调试回调函数
    glDebugMessageCallback(debugCallback, nullptr);

    // 在需要的地方使用 OpenGL 函数
    // ...

    return 0;
}
```
### 回调函数的参数说明
在 debugCallback 函数中，各个参数的含义如下：

- source：消息的来源（例如 GL_DEBUG_SOURCE_API 表示来自 API）
- type：消息的类型（例如 GL_DEBUG_TYPE_ERROR 表示错误）。
- id：消息的唯一标识符。
- severity：消息的严重性（例如 GL_DEBUG_SEVERITY_HIGH 表示严重错误）。
- message：消息内容字符串，包含具体的错误或警告信息。
### 控制调试信息输出
你可以使用 glDebugMessageControl 来过滤特定的消息类型，减少不必要的输出。
```cpp
// 只启用严重性为 HIGH 的调试信息
glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
```
### Hint
在vscode中调试，需要添加调试配置，在创建.vscode/launch.json:
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "C++ Debug (MSVC)",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}\\build\\MyGR.exe", // 替换为生成的可执行文件路径
            "args": [], // 启动时传递给程序的参数
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [], // 用于传递环境变量。可以为空或定义环境变量列表
            "console": "integratedTerminal", // 使用集成终端显示输出
            "preLaunchTask": "build_MyGL" // 设置为自动编译任务的名称
        }
    ]
}

```
同时注意往tasks.json的build_MyGL任务中添加参数以生成调试文件：
```json
"args": [
                ...
                "/Zi", // 需要调试时启用
                //"/Od", // 禁用优化，确保断点准确
                "/Fd:${workspaceFolder}\\build\\", // 指定.pdb文件路径
                ...
            ]
```


### 跟踪错误出现的行数
通过前面的步骤，我们已经能看到比较详细的错误信息了，但还是不能很直观的看到错误的位置。想要进一步优化，可以通过定义宏，使用GL_CALL()包裹 每一个/可能出错的 OpenGL函数
```cpp
#define GL_CALL(func) \
    func; \
    checkGLError(__FILE__, __LINE__)

void checkGLError(const char* file, int line) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error at " << file << ":" << line << " - " << gluErrorString(error) << std::endl;
    }
}
```


## 附录

### 2
glVertexAttribPointer 的第四个参数 GL_FALSE 或 GL_TRUE 用于指定是否将数据标准化。这个参数的作用在于，当顶点属性数据传递到着色器时，OpenGL 是否对其进行标准化处理。

**具体含义**
GL_FALSE：表示数据不标准化，按原值传递到着色器。

例如，如果数据类型是 GL_FLOAT，数据将按原样传递，不做任何更改。
如果数据类型是 GL_INT 或 GL_UNSIGNED_INT 等整数类型，同样按原值传递。
GL_TRUE：表示数据标准化，即将数据按类型的范围映射到 [0.0, 1.0]（无符号类型）或 [-1.0, 1.0]（有符号类型）。

如果数据类型是 GL_BYTE、GL_SHORT 等，则会根据它们的最小值和最大值进行映射。
例如，对于 GL_UNSIGNED_BYTE 类型的数据，范围 [0, 255] 将映射到 [0.0, 1.0]。
对于 GL_BYTE 类型的数据，范围 [-128, 127] 将映射到 [-1.0, 1.0]

### 3
GLuint和GL_UNSIGNED_INT
它们的值相同，但在代码中最好按语义进行区分，以避免混淆。比如：
使用 GLuint 来声明 OpenGL 对象的 ID 或句柄。
使用 GL_UNSIGNED_INT 来指定 OpenGL API 中的数据类型（例如 glVertexAttribPointer 等函数的参数）