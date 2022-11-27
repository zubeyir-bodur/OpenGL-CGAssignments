#version 150 core

// Vertex shader
#ifdef COMPILING_VS
layout(location = 0) in vec4 v_position;

uniform mat4 u_MVP;

void main()
{
	gl_Position = u_MVP * v_position;
	gl_Position.y = -gl_Position.y; // vertically flip the texture
}

// Pixel (fragment) shader
#elif defined (COMPILING_FS)

uniform uvec3 u_shape_model_id;
out uvec3 text_color;

void main()
{
	text_color = u_shape_model_id;
}
#endif