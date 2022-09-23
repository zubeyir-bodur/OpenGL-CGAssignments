#pragma once

unsigned int set_up_vertex_buffer(const float* vertex_buffer, unsigned int num_vertices, unsigned int num_coordinates_per_vertex);
unsigned int set_up_index_buffer(const unsigned int* index_buffer, unsigned int num_indices);
void shutdown(unsigned int program_id, unsigned int vertex_buffer_id, unsigned int index_buffer_id);