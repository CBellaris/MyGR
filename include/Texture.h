#pragma once
#include "stb_image.h"
#include <string>
#include <glad/glad.h>
#include <vector>
#include <iostream>
#include "Shader.h"
#include "GlobalSettings.h"

class Shader;

enum class TextureType {
    Diffuse,
    Specular,
    Normal,     // 法线贴图
    Metallic,   // 金属度贴图
    Roughness,  // 粗糙度贴图
    AO,         // 环境光遮蔽贴图
    Height,     // 高度贴图
    HDRI,       // HDR贴图
    Cubemap,    // 立方体贴图
    Noise,      // 噪声贴图
    BRDF,       // 预计算的Cook-Torrance BRDF贴图
    Prefilter,  // 预过滤的立方体贴图
    None    
};


struct TextureImage {
    std::string filePath;
    int width;
    int height;
    int channel;
    unsigned int textureID;
    TextureType type; 
};
 
class Texture {
private:
    std::vector<TextureImage> images;
    int maxTextureUnits;  // 存储显卡支持的最大纹理单元数
    const int MAX_TEXTURE_SLOTS_EACH_TYPE = GlobalSettings::getInstance().GetInt("MAX_TEXTURE_SLOTS_EACH_TYPE"); // 每种类型的最大纹理槽数
    static std::unordered_map<std::string, unsigned int> textureCache; // 纹理缓存
    
    // 纹理类型名称映射
    static std::unordered_map<TextureType, std::string> textureTypeNames;

    // 默认贴图
    std::unordered_map<TextureType, unsigned int> defaultTextures;

    float height_scale = 0.05f; // 高度贴图缩放因子

public:
    Texture();
    ~Texture();
    
    void add_image(const std::string& filePath, TextureType type, const unsigned char* rawData = nullptr, size_t size = 0); // 添加普通贴图
    void add_image_from_raw(const std::string& filePath, TextureType type, const unsigned char* rawData, int width, int height, int channel);   // 从内存添加图片
    void add_hdri(const std::string& filePath); // 添加HDR贴图
    void add_hdri_to_cubemap(const std::string& filePath, int resolution, bool prefilter = false);  // 添加HDR贴图并转换为立方体贴图
    void add_noise_texture();   // 添加噪声贴图
    void add_preCal_CT_BRDF(int resolution); // 添加预计算的Cook-Torrance BRDF贴图
    void bind(Shader* shader, TextureType type = TextureType::None, unsigned int binding_point = 0); // 绑定纹理到着色器，默认接收一个参数时，绑定所有纹理。额外两个参数可以指定绑定特定类型纹理
   // TEST
   unsigned int get_textureID(int index){return images[index].textureID;} 

private:
    unsigned char* read_image(TextureImage& image);
    unsigned char* read_image_from_memory(TextureImage& image, const unsigned char* rawData, size_t size);
    unsigned int load_hdr_texture(const std::string& filePath);
    GLuint convert_HDRI_to_cubemap(GLuint hdrTexture,int resolution);

    GLuint prefilter_cubemap(GLuint cubemap, int resolution, int cubemap_resolution); // 预过滤立方体贴图

    Shader* get_equirectangular_to_cubemap_shader();  
    Shader* get_prefilter_cubemap_shader(); // 预过滤立方体贴图着色器
    Shader* get_brdf_shader(); // 预计算Cook-Torrance BRDF贴图着色器

    void initialize_default_textures(); // 初始化默认贴图
};
