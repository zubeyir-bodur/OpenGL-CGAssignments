#pragma once

class VertexBuffer
{
private:
	unsigned int m_vertex_buffer_id;
	unsigned int m_size;
public:
	VertexBuffer();
	VertexBuffer(const void*, unsigned int);
	~VertexBuffer();

	void bind() const;
	void unbind() const;
	inline unsigned int size() const { return m_size; } ;
};
