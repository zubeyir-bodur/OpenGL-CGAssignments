#pragma once
#include <vector>

struct VertexBufferElement
{
	unsigned int type;
	unsigned int count;
	unsigned char normalized;

	static unsigned int get_size_of_type(unsigned int type);
};

class VertexBufferLayout
{
private:
	std::vector<VertexBufferElement> m_elements;
	unsigned int m_stride;
	unsigned int m_tot_elemets;
public:
	VertexBufferLayout() : m_stride(0), m_tot_elemets(0) {};

	/// <summary>
	/// Accept only registered templates
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="count"></param>
	template <typename T>
	void push_back_elements(unsigned int count);
	
	const std::vector<VertexBufferElement>& elements() const;
	unsigned int stride() const;
	unsigned int tot_elements() const;
};
