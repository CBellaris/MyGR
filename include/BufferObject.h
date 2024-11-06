# include <glad/glad.h>
# include <vector>

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

class VertexBufferLayout
{
private:
    std::vector<BufferLayoutElement> m_Elements;
    unsigned int m_Stride;
public:
    VertexBufferLayout(): m_Stride(0)
    {
    }
    ~VertexBufferLayout();
    
    template<typename T>
    void push(int count)
    {
        static_assert(sizeof(T) == 0, "push is not supported for this type");
    }

    template<>
    void push<float>(int count)
    {   
        BufferLayoutElement element = {GL_FLOAT, count, GL_FALSE};
        m_Elements.push_back(element);
        m_Stride += sizeof(GL_FLOAT);
    }

    void push<unsigned int>(int count)
    {   
        BufferLayoutElement element = {GL_UNSIGNED_INT, count, GL_FALSE};
        m_Elements.push_back(element);
        m_Stride += sizeof(GL_UNSIGNED_INT);
    }

    void push<unsigned char>(int count)
    {   
        BufferLayoutElement element = {GL_UNSIGNED_BYTE, count, GL_TRUE};
        m_Elements.push_back(element);
        m_Stride += sizeof(GL_UNSIGNED_BYTE);
    }

    inline unsigned int getStride() const {return m_Stride;};
    inline std::vector<BufferLayoutElement> getElement() const {return m_Elements;};
    
};

class VertexArrayObject
{
private:
    unsigned int m_RendererID;

public:
    VertexArrayObject();
    ~VertexArrayObject();

    void BindVertexBuffer();
    void BindIndexBuffer();

};