#version 150 core

// Vertex shader
#ifdef COMPILING_VS
layout(location = 0) in vec4 v_position;
layout(location = 1) in vec2 in_text_coord;

out vec2 v_text_coord;

uniform mat4 u_MVP;

void main()
{
	gl_Position = u_MVP * v_position;
	v_text_coord = in_text_coord;
}

// Pixel (fragment) shader
#elif defined (COMPILING_FS)
in vec2 v_text_coord;

uniform sampler2D u_texture;

void main()
{
	vec4 text_color = texture(u_texture, v_text_coord);
	gl_FragColor = text_color;
}
#endif