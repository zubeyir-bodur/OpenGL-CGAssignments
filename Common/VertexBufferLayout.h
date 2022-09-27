#pragma once
#include "glew.h"
#include "ErrorManager.h"
#include <vector>

struct VertexBufferElement
{
	unsigned int type;
	unsigned int count;
	unsigned char normalized;

	static unsigned int get_size_of_type(unsigned int type)
	{
		switch (type)
		{
			case GL_FLOAT:			return sizeof(float);
			case GL_UNSIGNED_INT:	return sizeof(unsigned int);
			case GL_UNSIGNED_BYTE:	return sizeof(unsigned char);
		}
		ASSERT(false);
		return 0;
	}
};

class VertexBufferLayout
{
private:
	std::vector<VertexBufferElement> m_elements;
	unsigned int m_stride;
public:
	VertexBufferLayout() : m_stride(0) {};

	/// <summary>
	/// Accept only registered templates below
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="count"></param>
	template <typename T>
	void push_back_elements(unsigned int count);

	template <>
	void push_back_elements<float>(unsigned int count)
	{
		m_elements.push_back({ GL_FLOAT, count, GL_FALSE });
		m_stride += VertexBufferElement::get_size_of_type(GL_FLOAT) * count;
	}

	template <>
	void push_back_elements<unsigned int>(unsigned int count)
	{
		m_elements.push_back({ GL_UNSIGNED_INT, count,GL_FALSE });
		m_stride += VertexBufferElement::get_size_of_type(GL_UNSIGNED_INT) * count;
	}

	template <>
	void push_back_elements<unsigned char>(unsigned int count)
	{
		m_elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		m_stride += VertexBufferElement::get_size_of_type(GL_UNSIGNED_BYTE) * count;
	}

	inline const std::vector<VertexBufferElement>& elements() const { return m_elements; }
	inline unsigned int stride() const { return m_stride; }
};