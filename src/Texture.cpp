#include "Texture.h"
#include "Mesh.h"
#include "GlobalSettings.h"
#include <algorithm> // For std::count_if

std::unordered_map<std::string, unsigned int> Texture::textureCache;

std::unordered_map<TextureType, std::string> Texture::textureTypeNames = {
    {TextureType::Diffuse, "diffuse"},
    {TextureType::Specular, "specular"},
    {TextureType::Normal, "normal"},
    {TextureType::Metallic, "metallic"},
    {TextureType::Roughness, "roughness"},
    {TextureType::AO, "ao"},
    {TextureType::Height, "height"},
    {TextureType::HDRI, "hdri"},
    {TextureType::Cubemap, "cubemap"},
    {TextureType::Noise, "noise"},
    {TextureType::BRDF, "brdfLUT"},
    {TextureType::Prefilter, "prefilterMap"}
};

Texture::Texture() {
    // 查询 GPU 最大支持的纹理单元数
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
    std::cout << "Max Texture Units Supported: " << maxTextureUnits << std::endl;

    initialize_default_textures(); // 初始化默认贴图
}

Texture::~Texture() {
    for (auto& image : images) {
        glDeleteTextures(1, &image.textureID);
    }
}

void Texture::add_image(const std::string& filePath, TextureType type, const unsigned char* rawData, size_t size) {
    // 检查当前类型的贴图数量
    int count = std::count_if(images.begin(), images.end(), [type](const TextureImage& img) {
        return img.type == type;
    });

    if (count >= MAX_TEXTURE_SLOTS_EACH_TYPE) {
        std::cerr << "Exceeded maximum texture slots for type: " << textureTypeNames[type] << std::endl;
        return;
    }

    // 检查缓存
    if (textureCache.find(filePath) != textureCache.end()) {
        TextureImage image = {filePath, 0, 0, 0, textureCache[filePath], type};
        images.push_back(image);
        return;
    }

    TextureImage image = {filePath, 0, 0, 0, 0, type};
    unsigned char* image_data = nullptr;

    // 读取图片数据
    if (rawData) {
        image_data = read_image_from_memory(image, rawData, size);
    } else {
        image_data = read_image(image);
    }
    if (!image_data) {
        std::cerr << "Failed to load image: " << filePath << std::endl;
        return;
    }

    // 生成 OpenGL 纹理
    glGenTextures(1, &image.textureID);
    glBindTexture(GL_TEXTURE_2D, image.textureID);

    // 设定纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = 0;
    // 选择合适的颜色通道格式
    if (image.channel == 4){
        format = GL_RGBA;
    } else if (image.channel == 3) {
        format = GL_RGB;
    } else if (image.channel == 1) {
        format = GL_RED;
    } else {
        std::cerr << "Unsupported channel count: " << image.channel << std::endl;
        stbi_image_free(image_data);
        return;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // 释放图像数据
    stbi_image_free(image_data);
    // 存储纹理信息
    images.push_back(image);
    textureCache[filePath] = image.textureID; // 存入缓存
}

void Texture::add_image_from_raw(const std::string &filePath, TextureType type, const unsigned char *rawData, int width, int height, int channel)
{
    // 检查当前类型的贴图数量
    int count = std::count_if(images.begin(), images.end(), [type](const TextureImage& img) {
        return img.type == type;
    });

    if (count >= MAX_TEXTURE_SLOTS_EACH_TYPE) {
        std::cerr << "Exceeded maximum texture slots for type: " << textureTypeNames[type] << std::endl;
        return;
    }

    // 检查缓存
    if (textureCache.find(filePath) != textureCache.end()) {
        TextureImage image = {filePath, width, height, channel, textureCache[filePath], type};
        images.push_back(image);
        return;
    }

    TextureImage image = {filePath, width, height, channel, 0, type};

    // 生成 OpenGL 纹理
    glGenTextures(1, &image.textureID);
    glBindTexture(GL_TEXTURE_2D, image.textureID);

    // 设定纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Assimp 默认通道为BGRA
    GLenum internalFormat = 0;
    GLenum inFormat = 0;
    // 选择合适的颜色通道格式
    if (image.channel == 4){
        internalFormat = GL_RGBA;
        inFormat =GL_BGRA;
    } else if (image.channel == 3) {
        internalFormat = GL_RGB;
        inFormat =GL_BGR;
    } else if (image.channel == 1) {
        internalFormat = GL_RED;
        inFormat =GL_RED;
    } else {
        std::cerr << "Unsupported channel count: " << image.channel << std::endl;
        return;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.width, image.height, 0, inFormat, GL_UNSIGNED_BYTE, rawData);
    glGenerateMipmap(GL_TEXTURE_2D);

    // 存储纹理信息
    images.push_back(image);
    textureCache[filePath] = image.textureID; // 存入缓存

}

void Texture::add_hdri(const std::string& filePath) {
    unsigned int hdrTexture = load_hdr_texture(filePath);
    if (hdrTexture) {
        TextureImage image = {filePath, 0, 0, 0, hdrTexture, TextureType::HDRI};
        images.push_back(image);
    }
}

void Texture::add_hdri_to_cubemap(const std::string &filePath, int resolution, bool prefilter)
{
    unsigned int hdrTexture = load_hdr_texture(filePath);
    GLuint cubemap = convert_HDRI_to_cubemap(hdrTexture, resolution);
    if (cubemap && prefilter) {
        GLuint prefilteredCubemap = prefilter_cubemap(cubemap, 512, resolution);
        TextureImage image = {"prefilterMap", 512, 512, 3, prefilteredCubemap, TextureType::Prefilter};
        images.push_back(image);
    } 
    
    if(cubemap) {
        TextureImage image = {filePath, resolution, resolution, 3, cubemap, TextureType::Cubemap};
        images.push_back(image);
    }

}

void Texture::add_noise_texture()
{
    // 检查缓存
    if (textureCache.find("noise") != textureCache.end()) {
        TextureImage image = {"noise", 4, 4, 1, textureCache["noise"], TextureType::Noise};
        images.push_back(image);
        return;
    }
    
    std::vector<glm::vec3> noise;
    const int noiseSize = 4;

    for (int i = 0; i < noiseSize * noiseSize; i++) {
        glm::vec3 n(
            (float)rand() / RAND_MAX * 2.0f - 1.0f, // x in [-1, 1]
            (float)rand() / RAND_MAX * 2.0f - 1.0f, // y in [-1, 1]
            0.0f
        );
        noise.push_back(glm::normalize(n));
    }

    TextureImage image = {"noise", 4, 4, 1, 0, TextureType::Noise};
    glGenTextures(1, &image.textureID);
    glBindTexture(GL_TEXTURE_2D, image.textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, noiseSize, noiseSize, 0, GL_RGB, GL_FLOAT, &noise[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    images.push_back(image);
    textureCache["noise"] = image.textureID; // 存入缓存

    glBindTexture(GL_TEXTURE_2D, 0); // 解绑纹理
}

void Texture::add_preCal_CT_BRDF(int resolution)
{
    //目前没有缓存机制，因为这个贴图只需要一张

    unsigned int brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);

    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, resolution, resolution, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 2. 创建FBO
    GLuint captureFBO;
    glGenFramebuffers(1, &captureFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    // 3. 使用渲染到纹理的方式来填充LUT纹理
    Shader* brdfShader = get_brdf_shader();
    Mesh quad = Mesh();
    quad.set_mesh_screen();

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, 512, 512);
    brdfShader->bind();
    glClear(GL_COLOR_BUFFER_BIT);
    quad.draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    // 清理
    glDeleteFramebuffers(1, &captureFBO);
    glViewport(0, 0, GlobalSettings::getInstance().GetInt("SCREEN_WIDTH"), GlobalSettings::getInstance().GetInt("SCREEN_HEIGHT"));

    TextureImage image = {"brdfLUT", resolution, resolution, 2, brdfLUTTexture, TextureType::BRDF};
    images.push_back(image);
}

void Texture::bind(Shader* shader, TextureType type, unsigned int binding_point) {
    if (images.empty()) {
        std::cerr << "No textures to bind." << std::endl;
        return;
    }

    shader->bind();

    // 单独绑定指定类型的纹理
    if (type != TextureType::None) {
        auto it = std::find_if(images.begin(), images.end(), [type](const TextureImage& img) {
            return img.type == type;
        });
        if (it != images.end()) {
            glActiveTexture(GL_TEXTURE0 + binding_point);
            if (it->type == TextureType::Cubemap || it->type == TextureType::Prefilter) {
                glBindTexture(GL_TEXTURE_CUBE_MAP, it->textureID);
            } else {
                glBindTexture(GL_TEXTURE_2D, it->textureID);
            }
            shader->setUniform1i("texture_" + textureTypeNames[type], binding_point); // 着色器中统一用 例如"texture_noise"
            if (type == TextureType::Noise) {
                shader->setUniform2f("noiseScale", GlobalSettings::getInstance().GetInt("SCREEN_WIDTH")/4.0f, GlobalSettings::getInstance().GetInt("SCREEN_HEIGHT")/4.0f);
            }
        } else {
            std::cerr << "No texture of type " << textureTypeNames[type] << " found." << std::endl;
        }
        return;
    }

    // 记录每种纹理类型的当前索引
    std::unordered_map<TextureType, int> textureCount;

    for (size_t i = 0; i < images.size(); i++) {
        // 单独绑定立方体贴图
        if (images[i].type == TextureType::Cubemap || images[i].type == TextureType::Prefilter) 
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_CUBE_MAP, images[i].textureID);
            // 获取当前类型的索引
            int index = textureCount[images[i].type]++;
            // 生成着色器采样器变量名，如 "texture_cubemap0", "texture_prefilterMap0"
            std::string texUniformName = "texture_" + textureTypeNames[images[i].type] + std::to_string(index); 
            shader->setUniform1i(texUniformName, i); 
        }
        else
        {
            // 绑定普通贴图
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, images[i].textureID);
            // 获取当前类型的索引
            int index = textureCount[images[i].type]++;
            // 生成着色器采样器变量名，如 "texture_diffuse0", "texture_specular1"
            std::string texUniformName = "texture_" + textureTypeNames[images[i].type] + std::to_string(index); 
            // 绑定到着色器
            shader->setUniform1i(texUniformName, i);
            // 如果是高度贴图，设置高度缩放因子
            if(images[i].type == TextureType::Height){
                shader->setUniform1f("height_scale", height_scale);
            }
        }   
    }

    int defaultCount = 0;
    // 为不足的槽位绑定默认贴图
    for (const auto& [type, defaultTextureID] : defaultTextures) {
        int count = textureCount[type];
        while (count < MAX_TEXTURE_SLOTS_EACH_TYPE) {
            int slot = images.size() + defaultCount;
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, defaultTextureID);
            std::string texUniformName = "texture_" + textureTypeNames[type] + std::to_string(count);
            shader->setUniform1i(texUniformName, slot);
            count++;
            defaultCount++;
            // 如果是高度贴图，设置高度缩放因子
            if(type == TextureType::Height){
                shader->setUniform1f("height_scale", height_scale);
            }
        }
    }
}

unsigned char* Texture::read_image(TextureImage& image) {
    stbi_set_flip_vertically_on_load(GlobalSettings::getInstance().GetBool("FLIP_VERTICAL_ON_LOAD")); // 是否翻转图片

    unsigned char* originalData = stbi_load(image.filePath.c_str(), &image.width, &image.height, &image.channel, 0);

    if (!originalData) {
        std::cerr << "Failed to load texture from: " << image.filePath << std::endl;
        return nullptr;
    }

    // (Deprecated)
    // // 如果是灰度图（单通道），扩展成 RGB，兼容 OpenGL
    // if (image.channel == 1) {
    //     int pixelCount = image.width * image.height;
    //     unsigned char* rgbData = new unsigned char[pixelCount * 3];
    //     for (int i = 0; i < pixelCount; ++i) {
    //         unsigned char gray = originalData[i];
    //         rgbData[i * 3 + 0] = gray;
    //         rgbData[i * 3 + 1] = gray;
    //         rgbData[i * 3 + 2] = gray;
    //     }

    //     stbi_image_free(originalData); // 释放原始灰度图
    //     image.channel = 3;
    //     return rgbData;
    // }

    // 非tga则保留原样（RGB 或 RGBA）
    return originalData;
}

unsigned char *Texture::read_image_from_memory(TextureImage &image, const unsigned char *rawData, size_t size)
{
    stbi_set_flip_vertically_on_load(GlobalSettings::getInstance().GetBool("FLIP_VERTICAL_ON_LOAD")); // 是否翻转图片

    unsigned char* imageData = stbi_load_from_memory(rawData, size, &image.width, &image.height, &image.channel, 0);
    if (!imageData) {
        std::cerr << "Failed to load image from memory\n";
        return false;
    }

    return imageData;
}

unsigned int Texture::load_hdr_texture(const std::string& filePath) {
    stbi_set_flip_vertically_on_load(GlobalSettings::getInstance().GetBool("FLIP_VERTICAL_ON_LOAD"));
    int width, height, nrComponents;
    float* data = stbi_loadf(filePath.c_str(), &width, &height, &nrComponents, 0);
    if (!data) {
        std::cerr << "Failed to load HDR image: " << filePath << std::endl;
        return 0;
    }

    unsigned int hdrTexture;
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return hdrTexture;
}

GLuint Texture::convert_HDRI_to_cubemap(GLuint hdrTexture, int resolution) {
    // 1. 创建立方体贴图
    GLuint cubemapTexture;
    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 2. 创建FBO
    GLuint captureFBO;
    glGenFramebuffers(1, &captureFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    // 3. 设置视角矩阵
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // 4. 使用着色器渲染到立方体贴图
    Shader* equirectangularToCubemapShader = get_equirectangular_to_cubemap_shader();
    equirectangularToCubemapShader->bind();
    equirectangularToCubemapShader->setUniform1i("equirectangularMap", 0);
    equirectangularToCubemapShader->setUniform4fv("projection", captureProjection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, resolution, resolution);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    Mesh cube = Mesh();
    cube.set_mesh_cube();
    for (unsigned int i = 0; i < 6; ++i) {
        equirectangularToCubemapShader->setUniform4fv("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemapTexture, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        cube.draw(); // 渲染立方体
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 清理
    glDeleteFramebuffers(1, &captureFBO);
    glViewport(0, 0, GlobalSettings::getInstance().GetInt("SCREEN_WIDTH"), GlobalSettings::getInstance().GetInt("SCREEN_HEIGHT"));

    return cubemapTexture;
}

GLuint Texture::prefilter_cubemap(GLuint cubemap, int resolution, int cubemap_resolution)
{
    unsigned int prefilterMap;
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // 2. 创建FBO
    GLuint captureFBO;
    glGenFramebuffers(1, &captureFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    // 3. 设置视角矩阵
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // 4. 使用着色器渲染到立方体贴图
    Shader* prefilterShader = get_prefilter_cubemap_shader();
    prefilterShader->bind();
    prefilterShader->setUniform4fv("projection", captureProjection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
    prefilterShader->setUniform1i("environmentMap", 0);
    prefilterShader->setUniform1i("resolution", cubemap_resolution);

    Mesh cube = Mesh();
    cube.set_mesh_cube();

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth  = resolution * std::pow(0.5, mip);
        unsigned int mipHeight = resolution * std::pow(0.5, mip);
        glViewport(0, 0, mipWidth, mipHeight);
    
        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader->setUniform1f("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            prefilterShader->setUniform4fv("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
    
            glClear(GL_COLOR_BUFFER_BIT);
            cube.draw(); // 渲染立方体
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 

    // 清理
    glDeleteFramebuffers(1, &captureFBO);
    glViewport(0, 0, GlobalSettings::getInstance().GetInt("SCREEN_WIDTH"), GlobalSettings::getInstance().GetInt("SCREEN_HEIGHT"));

    return prefilterMap;
}

Shader* Texture::get_equirectangular_to_cubemap_shader()
{
    static Shader* equirectangularToCubemapShader = nullptr;
    if (!equirectangularToCubemapShader)
    {
        equirectangularToCubemapShader = new Shader("res/shader/Equirectangular_to_cubemap.shader");
    }
    return equirectangularToCubemapShader;
}

Shader *Texture::get_prefilter_cubemap_shader()
{
    static Shader* prefilterShader = nullptr;
    if (!prefilterShader)
    {
        prefilterShader = new Shader("res/shader/Prefilter_cubemap.shader");
    }
    return prefilterShader;
}

Shader *Texture::get_brdf_shader()
{
    static Shader* brdfShader = nullptr;
    if (!brdfShader)
    {
        brdfShader = new Shader("res/shader/BRDF_LUT.shader");
    }
    return brdfShader;
}

void Texture::initialize_default_textures() {
    // 创建默认贴图
    unsigned char grayPixel[3] = {128, 128, 128}; // 半灰
    unsigned char bluePixel[3] = {128, 128, 255};     // 全蓝
    unsigned char heightPixel[1] = {0};         // 用于 Height（单通道）
    unsigned char aoPixel[1] = {255};         // 用于 ao（单通道）

    // Specular 默认贴图
    // 检查缓存
    if (textureCache.find("specular_deault") != textureCache.end()) {
        defaultTextures[TextureType::Specular] = textureCache["specular_deault"];
    } else{
        glGenTextures(1, &defaultTextures[TextureType::Specular]);
        glBindTexture(GL_TEXTURE_2D, defaultTextures[TextureType::Specular]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, grayPixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);   

        textureCache["specular_deault"] = defaultTextures[TextureType::Specular]; // 存入缓存
    }


    // Normal 默认贴图
    if (textureCache.find("normal_default") != textureCache.end()) {
        defaultTextures[TextureType::Normal] = textureCache["normal_default"];
    } else{
        glGenTextures(1, &defaultTextures[TextureType::Normal]);
        glBindTexture(GL_TEXTURE_2D, defaultTextures[TextureType::Normal]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, bluePixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        textureCache["normal_default"] = defaultTextures[TextureType::Normal]; // 存入缓存
    }

    // Height 默认贴图
    if (textureCache.find("height_default") != textureCache.end()) {
        defaultTextures[TextureType::Height] = textureCache["height_default"];
    } else{
        glGenTextures(1, &defaultTextures[TextureType::Height]);
        glBindTexture(GL_TEXTURE_2D, defaultTextures[TextureType::Height]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, heightPixel);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 可选：为 RED 通道设置采样行为，防止取不到正确的 r 分量
        GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

        textureCache["height_default"] = defaultTextures[TextureType::Height]; // 存入缓存
    }

    // AO 默认贴图
    if (textureCache.find("ao_default") != textureCache.end()) {
        defaultTextures[TextureType::AO] = textureCache["ao_default"];
    } else{
        glGenTextures(1, &defaultTextures[TextureType::AO]);
        glBindTexture(GL_TEXTURE_2D, defaultTextures[TextureType::AO]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, aoPixel);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 可选：为 RED 通道设置采样行为，防止取不到正确的 r 分量
        GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

        textureCache["ao_default"] = defaultTextures[TextureType::AO]; // 存入缓存
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}