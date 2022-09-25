#pragma once

class VertexBuffer
{
private:
	unsigned int m_vertex_buffer_id;
public:
	VertexBuffer();
	VertexBuffer(const void*, unsigned int);
	~VertexBuffer();

	void bind() const;
	void unbind() const;
};
