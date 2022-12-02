#version 150 core

// Vertex shader
#ifdef COMPILING_VS
layout(location = 0) in vec4 v_position;
layout(location = 1) in vec4 v_normal; // TODO - add computation for L, E, H
layout(location = 2) in vec2 v_text_coord;

out vec2 f_text_coord;

uniform mat4 u_MVP;

void main()
{
	gl_Position = u_MVP * v_position;
	f_text_coord = v_text_coord;
}

// Pixel (fragment) shader
#elif defined (COMPILING_FS)
in vec2 f_text_coord;
// TODO - add computation for ambient, diffuse and specular
uniform sampler2D u_texture;

void main()
{
	gl_FragColor = texture(u_texture, f_text_coord);
}
#endif