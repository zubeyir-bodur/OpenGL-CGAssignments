#include "Renderer/Renderer.h"
#include "Core/ErrorManager.h"
#include <glew.h>
#include <glfw3.h>

void Renderer::draw_triangles(const VertexArray* vertex_array_obj,
	const IndexBuffer* index_buffer_obj,
	const Shader* shader_obj)
{
	shader_obj->bind();
	vertex_array_obj->bind();
	index_buffer_obj->bind();
	__glCallVoid(glDrawElements(GL_TRIANGLES, index_buffer_obj->count(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::draw_polygon(const VertexArray* vertex_array_obj, const IndexBuffer* index_buffer_obj, const Shader* shader_obj)
{
	shader_obj->bind();
	vertex_array_obj->bind();
	index_buffer_obj->bind();
	__glCallVoid(glDrawElements(GL_TRIANGLE_FAN, index_buffer_obj->count(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::draw_lines(const VertexArray* vertex_array_obj,
	const IndexBuffer* index_buffer_obj,
	const Shader* shader_obj,
	int count,
	const void* offset)
{
	shader_obj->bind();
	vertex_array_obj->bind();
	index_buffer_obj->bind();
	if (count == -1 && offset == nullptr)
	{
		__glCallVoid(glDrawElements(GL_LINE_STRIP, index_buffer_obj->count(), GL_UNSIGNED_INT, nullptr));
	}
	else
	{
		__glCallVoid(glDrawElements(GL_LINE_LOOP, (unsigned int)count, GL_UNSIGNED_INT, offset));
	}
}

void Renderer::draw_seperate_lines(const VertexArray* vertex_array_obj, const IndexBuffer* index_buffer_obj, const Shader* shader_obj)
{
	shader_obj->bind();
	vertex_array_obj->bind();
	index_buffer_obj->bind();
	__glCallVoid(glDrawElements(GL_LINES, index_buffer_obj->count(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::clear(const float* clear_color)
{
	__glCallVoid(glClearColor(clear_color[0] * clear_color[3],
		clear_color[1] * clear_color[3],
		clear_color[2] * clear_color[3],
		clear_color[3]));
	__glCallVoid(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

void Renderer::clear()
{
	__glCallVoid(glClearColor(0.f, 0.f, 0.f, 0.f));
	__glCallVoid(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

void Renderer::set_viewport(GLFWwindow* window)
{
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	__glCallVoid(glViewport(0, 0, display_w, display_h));
}
