#pragma once

class IndexBuffer
{
private:
	unsigned int m_index_buffer_id;
	unsigned int m_count;
public:
	IndexBuffer();
	IndexBuffer(const unsigned int*, unsigned int);
	~IndexBuffer();

	void bind() const;
	void unbind() const;
	void draw() const;

	inline unsigned int count() const { return m_count; }
};
