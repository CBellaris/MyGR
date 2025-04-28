#include "Shader.h"

Shader::Shader(const std::string& filepath): m_FilePath(filepath), m_Program(0)
{
// 解析shader源码文件
    auto shadersCode = ParseShader();
    std::string vertexShaderCode = shadersCode[0].str();
    std::string fragmentShaderCode = shadersCode[1].str();
    std::string geometryShaderCode = shadersCode.size() > 2 ? shadersCode[2].str() : "";

    CreateShader(vertexShaderCode, fragmentShaderCode, geometryShaderCode);
}

Shader::~Shader()
{
    glDeleteProgram(m_Program);
}

void Shader::bind() const
{
    glUseProgram(m_Program);
}

void Shader::unbind() const
{
    glUseProgram(0);
}


int Shader::getUniformLocation(const std::string &name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
    {
        return m_UniformLocationCache[name];
    }
    int uLocation = glGetUniformLocation(m_Program, name.c_str());
    if (uLocation == -1)
    {
        std::cout<<"Warning: Uniform \""<<name<<"\" does not exist"<<std::endl;
    }

    m_UniformLocationCache[name] = uLocation;
    return uLocation;
}

void Shader::setUniform1f(const std::string &name, float v1)
{
    int loc = getUniformLocation(name);
    if (loc != -1)
        glUniform1f(loc, v1);
}

void Shader::setUniform2f(const std::string &name, float v1, float v2)
{
    int loc = getUniformLocation(name);
    if (loc != -1)
        glUniform2f(loc, v1, v2);
}

void Shader::setUniform3f(const std::string &name, float v1, float v2, float v3)
{
    int loc = getUniformLocation(name);
    if (loc != -1)
        glUniform3f(loc, v1, v2, v3);
}

void Shader::setUniform4f(const std::string &name, float v1, float v2, float v3, float v4)
{
    int loc = getUniformLocation(name);
    if (loc != -1)
        glUniform4f(loc, v1, v2, v3, v4);
}

void Shader::setUniform1i(const std::string &name, int v1)
{
    int loc = getUniformLocation(name);
    if (loc != -1)
        glUniform1i(loc, v1);
}

void Shader::setUniform1iv(const std::string& name, int count, const int* values)
{
    int loc = getUniformLocation(name);
    if (loc != -1)
        glUniform1iv(loc, count, values);
}

void Shader::setUniform3fv(const std::string &name, int count, const float *values)
{
    int loc = getUniformLocation(name);
    if (loc != -1)
        glUniform3fv(loc, count, values);
}

void Shader::setUniform4fv(const std::string &name, glm::mat4& mat)
{
    int loc = getUniformLocation(name);
    if (loc != -1)
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

std::vector<std::stringstream> Shader::ParseShader()
{
    std::ifstream stream(m_FilePath);

    enum class ShaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1,
        GEOMETRY = 2
    };

    ShaderType type = ShaderType::NONE;
    std::string line;
    std::vector<std::stringstream> ss(3); // Support for 3 shader types
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                type = ShaderType::FRAGMENT;
            }
            else if (line.find("geometry") != std::string::npos)
            {
                type = ShaderType::GEOMETRY;
            }
        }
        else
        {
            if (type != ShaderType::NONE)
            {
                ss[static_cast<int>(type)] << line << '\n';
            }
        }
    }

    return ss;
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    // 获取异常
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = new char[length];
        glGetShaderInfoLog(id, length, &length, message);

        // 根据着色器类型输出错误信息
        std::string shaderType;
        if (type == GL_VERTEX_SHADER)
            shaderType = "VertexShader";
        else if (type == GL_FRAGMENT_SHADER)
            shaderType = "FragmentShader";
        else if (type == GL_GEOMETRY_SHADER)
            shaderType = "GeometryShader";
        else
            shaderType = "UnknownShader";

        std::cout << "Failed to compile the " << shaderType << " of " << m_FilePath << ": " << message << std::endl;
        delete[] message;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

void Shader::CreateShader(const std::string& VertexShader, const std::string& FragmentShader, const std::string& GeometryShader)
{
    m_Program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, VertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, FragmentShader);
    unsigned int gs = 0;

    if (!GeometryShader.empty())
    {
        gs = CompileShader(GL_GEOMETRY_SHADER, GeometryShader);
        glAttachShader(m_Program, gs);
    }

    glAttachShader(m_Program, vs);
    glAttachShader(m_Program, fs);
    glLinkProgram(m_Program);
    glValidateProgram(m_Program);

    //获取链接异常
    int isLinked;
    glGetProgramiv(m_Program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        int length;
        glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &length);
        char* message = new char[length];
        glGetProgramInfoLog(m_Program, length, &length, message);
        std::cout << "Failed to link program of " << m_FilePath << ": "<< message << std::endl;
        delete[] message;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    if (gs != 0)
    {
        glDeleteShader(gs);
    }
}