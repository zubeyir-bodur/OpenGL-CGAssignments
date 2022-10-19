#version 150 core

// Vertex shader
#ifdef COMPILING_VS
layout(location = 0) in vec4 v_position;

uniform mat4 u_MVP;

void main()
{
	gl_Position = u_MVP * v_position;
}

// Pixel (fragment) shader
#elif defined (COMPILING_FS)

uniform vec4 u_color;

void main()
{
	gl_FragColor = u_color;
}
#endif