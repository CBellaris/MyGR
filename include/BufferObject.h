#pragma once
#include <glad/glad.h>
#include <vector>
#include <iostream>
#include "GlobalSettings.h"

namespace {

class VertexBuffer
{
private:
    unsigned int m_RendererID;
    unsigned int m_Count;
public:
    // 模板函数必须放在头文件中
    template <typename T>
    VertexBuffer(const std::vector<T>& data)
    {
        size_t size = data.size() * sizeof(T);
        // 创建顶点缓冲对象(Vertex Buffer Objects, VBO)
        glGenBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);  
        // 把用户定义的数据复制到当前绑定缓冲, GL_STATIC_DRAW ：数据不会或几乎不会改变; GL_DYNAMIC_DRAW：数据会被改变很多; GL_STREAM_DRAW ：数据每次绘制时都会改变
        glBufferData(GL_ARRAY_BUFFER, size, data.data(), GL_STATIC_DRAW);
    }

    ~VertexBuffer();

    void bind();
    void unbind();
};

class IndexBuffer
{
private:
    unsigned int m_RendererID;
    unsigned int m_Count;
public:
    IndexBuffer(const std::vector<unsigned int>& data);
    ~IndexBuffer();

    void bind();
    void unbind();
    inline unsigned int getCount() const;
};



struct BufferLayoutElement
{
    unsigned int type;
    unsigned int count;
    bool normalized;
};

// 用于管理顶点数据的排列结构
class VertexBufferLayout
{
private:
    std::vector<BufferLayoutElement> m_Elements;
    unsigned int m_Stride;
public:
    VertexBufferLayout(): m_Stride(0)
    {
    }
    ~VertexBufferLayout()
    {
    }
    
    template<typename T>
    void push(unsigned int count)
    {
        static_assert(sizeof(T) == 0, "push is not supported for this type");
    }

    template<>
    void push<float>(unsigned int count)
    {   
        BufferLayoutElement element = {GL_FLOAT, count, GL_FALSE};
        m_Elements.push_back(element);
        m_Stride += count * sizeof(GL_FLOAT);
    }

    template<>
    void push<unsigned int>(unsigned int count)
    {   
        BufferLayoutElement element = {GL_UNSIGNED_INT, count, GL_FALSE};
        m_Elements.push_back(element);
        m_Stride += count * sizeof(GL_UNSIGNED_INT);
    }

    template<>
    void push<unsigned char>(unsigned int count)
    {   
        BufferLayoutElement element = {GL_UNSIGNED_BYTE, count, GL_TRUE};
        m_Elements.push_back(element);
        m_Stride += count * sizeof(GL_UNSIGNED_BYTE);
    }

    inline unsigned int get_stride() const {return m_Stride;};
    inline const std::vector<BufferLayoutElement>& get_element() const {return m_Elements;};
    
};
}

class VertexArrayObject
{
private:
    unsigned int m_RendererID;
    VertexBuffer* vb;
    IndexBuffer* ib;
    VertexBufferLayout m_Layout;

public:
    VertexArrayObject();
    ~VertexArrayObject();

    template<typename T>
    void addVertexBuffer(const std::vector<T>& data)
    {
        vb = new VertexBuffer(data);
    }

    void addIndexBuffer(const std::vector<unsigned int>& data);


    template<typename T>
    void push(unsigned int count)
    {
        m_Layout.push<T>(count);
    }

    void setLayout();

    /**
    * @brief 调用以完成对此VAO的绑定
    */
    void bindAll();


    void bind();
    void unbind();
    
};

// 使用示例
    // // 定义顶点数据
    // std::vector<float> vertices = {
    //     -0.5f,-0.5f, 0.0f, 0.0f,
    //     0.5f, -0.5f, 1.0f, 0.0f,
    //     0.5f,  0.5f, 1.0f, 1.0f,
    //     -0.5f, 0.5f, 0.0f, 1.0f
    // };

    // // 定义索引数组
    // std::vector<unsigned int> indices = {
    //     0, 1, 2,
    //     2, 3, 0
    // };


    // // 使用自定义的类，管理一个VAO，并绑定VBO和IBO
    // VertexArrayObject* VAO = new VertexArrayObject();
    // VAO->addVertexBuffer(vertices);
    // VAO->addIndexBuffer(indices);
    // VAO->push<float>(2);
    // VAO->push<float>(2);
    // VAO->bindAll();
    // VAO->bind();



template <typename T>
class SSBO {
public:
    /**
     * @brief 构造函数，创建一个 Shader Storage Buffer Object (SSBO)
     * @param bindingPoint 绑定点索引
     * @param maxDataSize 最大数据容量（元素数量）
     */
    SSBO(GLuint bindingPoint, unsigned int maxDataSize);
    
    /**
     * @brief 析构函数，释放 SSBO 资源
     */
    ~SSBO();

    /**
     * @brief 更新 SSBO 的数据
     * @param data 需要更新的数据
     * @param offset 数据在 SSBO 中的偏移量（字节）
     * @throws std::runtime_error 如果数据超出缓冲区大小
     */
    void updateData(const std::vector<T>& data, size_t offset = 0);
    
    void bind() const;
    void unbind() const;

private:
    GLuint ssbo; ///< SSBO 句柄
    GLuint bindingPoint; ///< 绑定点索引
    size_t bufferSize; ///< SSBO 分配的缓冲区大小（字节）
};

template <typename T>
SSBO<T>::SSBO(GLuint bindingPoint, unsigned int maxDataSize) : bindingPoint(bindingPoint){
    bufferSize = maxDataSize * sizeof(T);
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

template <typename T>
SSBO<T>::~SSBO() {
    glDeleteBuffers(1, &ssbo);
}

template <typename T>
void SSBO<T>::updateData(const std::vector<T>& data, size_t offset) {
    size_t dataSize = data.size() * sizeof(T);
    if (offset + dataSize > bufferSize) {
        throw std::runtime_error("SSBO::updateData: Data exceeds buffer size");
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, dataSize, data.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

template <typename T>
void SSBO<T>::bind() const {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
}

template <typename T>
void SSBO<T>::unbind() const {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}



class UBO {
public:
    UBO(size_t size, GLuint bindingPoint) 
        : uboSize(size), binding(bindingPoint), offset(0) {
        glGenBuffers(1, &uboID);
        glBindBuffer(GL_UNIFORM_BUFFER, uboID);
        glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW); // 预分配内存
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // 绑定到指定的 UBO 绑定点
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, uboID);
    }

    ~UBO() {
        glDeleteBuffers(1, &uboID);
    }

    void Bind() const {
        glBindBuffer(GL_UNIFORM_BUFFER, uboID);
    }

    void Unbind() const {
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    // 直接向 UBO 存储数据（自动管理偏移量）
    template<typename T>
    void PushData(const T& data) {
        size_t dataSize = sizeof(T);
        if (offset + dataSize > uboSize) {
            std::cerr << "Error: UBO overflow! Cannot push more data." << std::endl;
            return;
        }
        UpdateData(&data, dataSize, offset);
        offset += AlignOffset(dataSize); // 自动对齐
    }

    // 更新指定偏移量的数据
    void UpdateData(const void* data, size_t size, size_t customOffset) {
        Bind();
        glBufferSubData(GL_UNIFORM_BUFFER, customOffset, size, data);
        Unbind();
    }

    // 设置新的 UBO 绑定点
    void SetBindingPoint(GLuint newBindingPoint) {
        binding = newBindingPoint;
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, uboID);
    }

    GLuint GetID() const { return uboID; }

private:
    GLuint uboID;
    GLuint binding;
    size_t uboSize;
    size_t offset; // 记录当前 UBO 已使用的偏移量

    // 确保数据的内存对齐（std140 规范）
    static size_t AlignOffset(size_t size) {
        const size_t alignment = 16; // std140 规范：数据对齐到 16 字节
        return (size + alignment - 1) & ~(alignment - 1);
    }
};


class Framebuffer {
    public:
        struct AttachmentConfig {
            GLenum internalFormat;
            GLenum format;
            GLenum type;
        };
    
        Framebuffer(int width, int height, const std::vector<AttachmentConfig>& attachments, bool useDepth = true, bool useStencil = true)
            : width(width), height(height), useDepth(useDepth), useStencil(useStencil), colorAttachments(attachments.size()) {
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
            // 创建颜色附件
            textures.resize(colorAttachments);
            glGenTextures(colorAttachments, textures.data());
            for (size_t i = 0; i < attachments.size(); ++i) {
                glBindTexture(GL_TEXTURE_2D, textures[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, attachments[i].internalFormat, width, height, 0, attachments[i].format, attachments[i].type, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);
            }
    
            // 处理多个颜色附件
            if (colorAttachments > 1) {
                std::vector<GLenum> drawBuffers;
                for (int i = 0; i < colorAttachments; ++i) {
                    drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
                }
                glDrawBuffers(drawBuffers.size(), drawBuffers.data());
            }
    
            // 创建深度和模板缓冲
            if (useDepth || useStencil) {
                glGenRenderbuffers(1, &rbo);
                glBindRenderbuffer(GL_RENDERBUFFER, rbo);
                if (useDepth && useStencil) {
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
                } else if (useDepth) {
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
                }
            }
    
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "ERROR: Framebuffer is not complete!" << std::endl;
            }
    
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    
        ~Framebuffer() {
            glDeleteFramebuffers(1, &fbo);
            glDeleteTextures(textures.size(), textures.data());
            if (rbo) {
                glDeleteRenderbuffers(1, &rbo);
            }
        }
    
        void bind() const {
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        }
    
        void unbind() const {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    
        // 获取帧缓冲的结果
        GLuint get_texture(int index = 0) const {
            if (index >= 0 && index < textures.size()) {
                return textures[index];
            }
            return 0;
        }
    
        void switch_depth_component_to_default() {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // 写入到默认帧缓冲
            glBlitFramebuffer(
              0, 0, GlobalSettings::getInstance().GetInt("SCREEN_WIDTH"), GlobalSettings::getInstance().GetInt("SCREEN_HEIGHT"), 0, 0, GlobalSettings::getInstance().GetInt("SCREEN_WIDTH"), GlobalSettings::getInstance().GetInt("SCREEN_HEIGHT"), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // 直接重新创建会更方便，这个用不上
        void resize(int newWidth, int newHeight, const std::vector<AttachmentConfig>& attachments) {
            width = newWidth;
            height = newHeight;
    
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
            for (size_t i = 0; i < attachments.size(); ++i) {
                glBindTexture(GL_TEXTURE_2D, textures[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, attachments[i].internalFormat, width, height, 0, attachments[i].format, attachments[i].type, nullptr);
            }
    
            if (rbo) {
                glBindRenderbuffer(GL_RENDERBUFFER, rbo);
                if (useDepth && useStencil) {
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
                } else if (useDepth) {
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
                }
            }
    
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    
    private:
        GLuint fbo;
        std::vector<GLuint> textures;
        GLuint rbo = 0;
        int width, height;
        bool useDepth, useStencil;
        int colorAttachments;
    };
