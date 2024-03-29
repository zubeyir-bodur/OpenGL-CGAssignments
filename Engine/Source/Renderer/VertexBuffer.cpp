#include "Renderer/VertexBuffer.h"
#include "Core/ErrorManager.h"
#include <glew.h>

VertexBuffer::VertexBuffer() :
	m_size(0)
{
	__glCallVoid(glGenBuffers(1, &m_vertex_buffer_id));
}

VertexBuffer::VertexBuffer(const void* data, unsigned int size) :
	m_size(size)
{
	__glCallVoid(glGenBuffers(1, &m_vertex_buffer_id));
	__glCallVoid(glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id));
	__glCallVoid(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer()
{
	__glCallVoid(glDeleteBuffers(1, &m_vertex_buffer_id));
}

void VertexBuffer::bind() const
{
	__glCallVoid(glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id));
}

void VertexBuffer::unbind() const
{
	__glCallVoid(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

