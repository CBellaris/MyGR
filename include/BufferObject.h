# include <glad/glad.h>
# include <vector>

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

    inline unsigned int getStride() const {return m_Stride;};
    inline const std::vector<BufferLayoutElement>& getElement() const {return m_Elements;};
    
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
    * @brief 调用完成对此VAO的绑定
    */
    void bindAll();


    void bind();
    void unbind();
    
};