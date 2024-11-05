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