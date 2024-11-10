#pragma once
#include "stb_image.h"
#include <string>
#include <glad/glad.h>
#include <vector>
#include <iostream>
#include "Shader.h"

class Shader;

struct TextureImage
{
    /* data */
    std::string filePath;
    int height;
    int width;
    int channel;
};


class Texture
{
private:
    std::vector<unsigned int> m_TextureID;
    std::vector<TextureImage> images;

public:
    Texture();
    ~Texture();
    void add_image(const std::string& filePath);

    void bind(Shader* shader);

private:
    unsigned char* read_image(int idx);
};