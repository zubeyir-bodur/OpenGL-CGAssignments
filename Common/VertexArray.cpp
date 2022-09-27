#include "VertexArray.h"
#include "ErrorManager.h"
#include "glew.h"

VertexArray::VertexArray()
{
	__glCallVoid(glGenVertexArrays(1, &m_vertex_array_id));
	__glCallVoid(glBindVertexArray(m_vertex_array_id));
}

VertexArray::~VertexArray()
{
	__glCallVoid(glDeleteVertexArrays(1, &m_vertex_array_id));
}

void VertexArray::add_buffer(const VertexBuffer& vertex_buffer, const VertexBufferLayout& layout) const
{
	bind();
	vertex_buffer.bind();
	const auto& elements = layout.elements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		__glCallVoid(glEnableVertexAttribArray(i));
		__glCallVoid(glVertexAttribPointer(i,
			element.count,
			element.type,		//type
			element.normalized,	// normalized flag
			layout.stride(),	// stride size, in bytes
			(const void*)offset	// offset to the first item of the next attribute
		));
		offset += element.count * VertexBufferElement::get_size_of_type(element.type);
	}
}

void VertexArray::bind() const
{
	__glCallVoid(glBindVertexArray(m_vertex_array_id));
}

void VertexArray::unbind() const
{

	__glCallVoid(glBindVertexArray(0));
}
