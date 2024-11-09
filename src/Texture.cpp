#include "Texture.h"

Texture::Texture(): m_TextureID(1)
{
}

Texture::~Texture()
{
    for (int i=0; i<m_TextureID.size(); i++)
        glDeleteTextures(1, &m_TextureID[i]);
}

void Texture::add_image(const std::string& filePath)
{
    TextureImage image = {
        filePath,
        0,
        0,
        0
    };

    images.push_back(image);
}

void Texture::bind(Shader* shader)
{
    for (int i=0; i<images.size(); i++)
    {   
        shader->bind();
        glActiveTexture(GL_TEXTURE0+i);

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // 为当前绑定的纹理对象设置环绕、过滤方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        m_TextureID.push_back(texture);

        unsigned char* image_data = read_image(i);

        TextureImage& image = images[i];
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width, image.height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
        glGenerateMipmap(GL_TEXTURE_2D);

        std::string texUniformName = "texture";
        texUniformName += std::to_string(i + 1);
        shader->setUniform1i(texUniformName, i);

        stbi_image_free(image_data);
        
    }
}

unsigned char* Texture::read_image(int idx)
{
    TextureImage& image = images[idx];
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(image.filePath.c_str(), &image.width, &image.height, &image.channel, 0);
    if (!data)
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    return data;
}
