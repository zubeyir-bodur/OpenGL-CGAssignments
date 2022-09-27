#version 150 core

// Vertex shader
#ifdef COMPILING_VS
layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texture_coord;

out vec2 v_text_coord;

void main()
{
	gl_Position = position;
	v_text_coord = texture_coord;
}

// Pixel (fragment) shader
#elif defined (COMPILING_FS)
layout(location = 0) out vec4 color;

in vec2 v_text_coord;

uniform vec4 u_color;
uniform sampler2D u_texture;

void main()
{
	vec4 text_color = texture(u_texture, v_text_coord);
	color = text_color + u_color;
}
#endif