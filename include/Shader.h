#pragma once

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <vector>
#include <glad/glad.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera;
class Light;

class Shader
{
private:
    unsigned int m_Program;
    std::string m_FilePath;
    std::unordered_map<std::string, int> m_UniformLocationCache; // 缓存全局变量位置
public:
    Shader(const std::string& filepath);
    ~Shader();

    void bind() const;
    void unbind() const;

    // set light
    //void set_light(Light* light);

    // set uniforms
    int getUniformLocation(const std::string& name);  

    void setUniform1f(const std::string& name, float v1);
    void setUniform2f(const std::string& name, float v1, float v2);
    void setUniform3f(const std::string& name, float v1, float v2, float v3);
    void setUniform4f(const std::string& name, float v1, float v2, float v3, float v4);
    void setUniform1i(const std::string& name, int v1);
    void setUniform1iv(const std::string& name, int count, const int* values); 
    void setUniform3fv(const std::string& name, int count, const float* values);
    void setUniform4fv(const std::string& name, glm::mat4& mat);

private:
    // 解析.shader文件并编译着色器
    std::vector<std::stringstream> ParseShader();
    unsigned int CompileShader(unsigned int type, const std::string& source);
    void CreateShader(const std::string& VertexShader, const std::string& FragmentShader, const std::string& GeometryShader = "");
};