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

