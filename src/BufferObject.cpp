#include "BufferObject.h"

//----------------------------
VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &m_RendererID);
}

void VertexBuffer::bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);  
}

void VertexBuffer::unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);  
}
// ------------------------------------------------------------
IndexBuffer::IndexBuffer(const std::vector<unsigned int>& data)
{
    // 创建索引缓冲对象(Index Buffer Object，IBO)
    _ASSERT(sizeof(unsigned int) == sizeof(GLuint));
    size_t size = data.size() * sizeof(unsigned int);
    m_Count = data.size();
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data.data(), GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer()
{
    glDeleteBuffers(1, &m_RendererID);
}

void IndexBuffer::bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
}

void IndexBuffer::unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

inline unsigned int IndexBuffer::getCount() const
{
    return m_Count;
}

//-------------------------------------------------

VertexArrayObject::VertexArrayObject()
{
    // 创建顶点数组对象(Vertex Array Object, VAO)
    glGenVertexArrays(1, &m_RendererID);
    glBindVertexArray(m_RendererID);
}

VertexArrayObject::~VertexArrayObject()
{
    delete vb;
    delete ib;
    glDeleteVertexArrays(1, &m_RendererID);
}

void VertexArrayObject::addIndexBuffer(const std::vector<unsigned int>& data)
{
    ib = new IndexBuffer(data);
}

void VertexArrayObject::setLayout()
{
    const std::vector<BufferLayoutElement>& elements = m_Layout.get_element();
    unsigned int offset = 0;
    for(int i = 0; i < elements.size(); i++)
    {
        const auto& element = elements[i];
        // 启用属性
        glEnableVertexAttribArray(i);
        // 指定属性的格式(第几个属性，包含几个数据，数据类型，是否标准化，一个顶点的步长，到下一个属性的指针)
        glVertexAttribPointer(i, element.count, element.type, element.normalized, m_Layout.get_stride(), (const void*)(uintptr_t)offset);
        offset += sizeof(element.type) * element.count;
    }
}

void VertexArrayObject::bindAll()
{
    setLayout();
    glBindVertexArray(0);
}

void VertexArrayObject::bind()
{
    glBindVertexArray(m_RendererID);
}

void VertexArrayObject::unbind()
{
    glBindVertexArray(0);
}
