#version 150 core

// Vertex shader
#ifdef COMPILING_VS
layout(location = 0) in vec4 v_position;
layout(location = 1) in vec4 v_color;

out vec4 f_color;

uniform mat4 u_MVP;

void main()
{
	gl_Position = u_MVP * v_position;
	f_color = v_color;
}

// Pixel (fragment) shader
#elif defined (COMPILING_FS)
in vec4 f_color;

void main()
{
	gl_FragColor = f_color;
}
#endif