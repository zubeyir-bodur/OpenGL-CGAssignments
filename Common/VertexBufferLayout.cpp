#include "ErrorManager.h"
#include "VertexBufferLayout.h"
#include "glew.h"

unsigned int VertexBufferElement::get_size_of_type(unsigned int type)
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

template <>
void VertexBufferLayout::push_back_elements<float>(unsigned int count)
{
	m_elements.push_back({ GL_FLOAT, count, GL_FALSE });
	m_stride += VertexBufferElement::get_size_of_type(GL_FLOAT) * count;
	m_tot_elemets += count;
}

template <>
void VertexBufferLayout::push_back_elements<unsigned int>(unsigned int count)
{
	m_elements.push_back({ GL_UNSIGNED_INT, count,GL_FALSE });
	m_stride += VertexBufferElement::get_size_of_type(GL_UNSIGNED_INT) * count;
	m_tot_elemets += count;
}

template <>
void VertexBufferLayout::push_back_elements<unsigned char>(unsigned int count)
{
	m_elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
	m_stride += VertexBufferElement::get_size_of_type(GL_UNSIGNED_BYTE) * count;
	m_tot_elemets += count;
}

const std::vector<VertexBufferElement>& VertexBufferLayout::elements() const { return m_elements; }
unsigned int VertexBufferLayout::stride() const { return m_stride; }
unsigned int VertexBufferLayout::tot_elements() const { return m_tot_elemets; };


