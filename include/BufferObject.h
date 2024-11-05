# include <glad/glad.h>
# include <vector>

class VertexBuffer
{
private:
    unsigned int m_RendererID;
    unsigned int m_Count;
public:
    template <typename T>
    VertexBuffer(const std::vector<T>& data);
    ~VertexBuffer();

    void Bind();
    void Unbind();
};

class IndexBuffer
{
private:
    unsigned int m_RendererID;
    unsigned int m_Count;
public:
    IndexBuffer(const std::vector<unsigned int>& data);
    ~IndexBuffer();

    void Bind();
    void Unbind();
    inline unsigned int getCount() const;
};