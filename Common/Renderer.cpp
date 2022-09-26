#include "Renderer.h"
#include "ThirdParty/GLFW/include/glfw3.h"

void Renderer::draw(const VertexArray* vertex_array_obj,
	const IndexBuffer* index_buffer_obj,
	const Shader* shader_obj) const
{
	shader_obj->bind();
	vertex_array_obj->bind();
	index_buffer_obj->bind();
	__glCallVoid(glDrawElements(GL_TRIANGLES, index_buffer_obj->count(), GL_UNSIGNED_INT, nullptr));

}

void Renderer::clear(const float* clear_color) const
{
	__glCallVoid(glClearColor(clear_color[0] * clear_color[3],
		clear_color[1] * clear_color[3],
		clear_color[2] * clear_color[3],
		clear_color[3]));
	__glCallVoid(glClear(GL_COLOR_BUFFER_BIT));
}

void Renderer::set_viewport(GLFWwindow* window) const
{
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	__glCallVoid(glViewport(0, 0, display_w, display_h));
}
