#pragma once
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

class VertexArray
{
	unsigned int m_vertex_array_id;
private:

public:
	VertexArray();
	~VertexArray();

	void add_buffer(const VertexBuffer& vertex_buffer, const VertexBufferLayout& layout) const; 
	void bind() const;
	void unbind() const;
};