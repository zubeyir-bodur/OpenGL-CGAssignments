#include "IndexBuffer.h"
#include "ErrorManager.h"
#include "glew.h"
#include "GL/GL.h"

IndexBuffer::IndexBuffer() : m_count(0)
{
	__glCallVoid(glGenBuffers(1, &m_index_buffer_id));
}

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count)
	: m_count(count)
{
	ASSERT(sizeof(unsigned int) == sizeof(GLuint));
	__glCallVoid(glGenBuffers(1, &m_index_buffer_id));
	__glCallVoid(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer_id));
	__glCallVoid(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer()
{
	__glCallVoid(glDeleteBuffers(1, &m_index_buffer_id));
}

void IndexBuffer::bind() const
{
	__glCallVoid(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer_id));
}

void IndexBuffer::unbind() const
{
	__glCallVoid(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
