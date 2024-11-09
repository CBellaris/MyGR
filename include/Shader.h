#pragma once

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <vector>
#include <glad/glad.h>
#include <iostream>

class Shader
{
private:
    unsigned int m_Program;
    std::string m_FilePath;
    std::unordered_map<std::string, int> m_UniformLocationCache;
public:
    Shader(const std::string& filepath);
    ~Shader();

    void bind() const;
    void unbind() const;

    // set uniforms
    int getUniformLocation(const std::string& name);

    void setUniform4f(const std::string& name, float v1, float v2, float v3, float v4);
    void setUniform1i(std::string& name, int v1);

private:
    std::vector<std::stringstream> ParseShader();
    unsigned int CompileShader(unsigned int type, const std::string& source);
    void CreateShader(const std::string& VertexShader, const std::string& FragmentShader);
};