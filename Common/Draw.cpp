#include "Draw.h"
#include "ErrorManager.h"
#include "glew.h"
#include "GL/GL.h"

unsigned int set_up_vertex_buffer(const float* vertex_buffer, unsigned int num_vertices, unsigned int num_coordinates_per_vertex)
{
	unsigned int start_index = 0;
    unsigned int vertex_buffer_id;
	glCallVoid(glGenBuffers(1, &vertex_buffer_id));
	glCallVoid(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id));
	glCallVoid(glBufferData(GL_ARRAY_BUFFER, num_vertices * num_coordinates_per_vertex * sizeof(float), vertex_buffer, GL_STATIC_DRAW));

	// Tell OpenGL that the vertex buffer has 2 dimensions
	glCallVoid(glEnableVertexAttribArray(start_index));
	glCallVoid(glVertexAttribPointer(start_index,
		num_coordinates_per_vertex,
		GL_FLOAT, //type
		GL_FALSE, // normalized flag
		sizeof(float) * num_coordinates_per_vertex, // stride size, in bytes
		nullptr // pointer to the next attribute
	));
    return vertex_buffer_id;
}

unsigned int set_up_index_buffer(const unsigned int* index_buffer, unsigned int num_indices)
{
	unsigned int index_buffer_id;
	glCallVoid(glGenBuffers(1, &index_buffer_id));
	glCallVoid(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id));
	glCallVoid(glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(unsigned int), index_buffer, GL_STATIC_DRAW));
	return index_buffer_id;
}

void shutdown(unsigned int program_id, unsigned int vertex_buffer_id, unsigned int index_buffer_id)
{
	glCallVoid(glDeleteProgram(program_id));
	glCallVoid(glDeleteBuffers(1, &vertex_buffer_id));
	glCallVoid(glDeleteBuffers(1, &index_buffer_id));
}
