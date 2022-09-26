#version 150 core

// Vertex shader
#ifdef COMPILING_VS
layout(location = 0) in vec4 position;

void main()
{
	gl_Position = position;
}

// Pixel (fragment) shader
#elif defined (COMPILING_FS)
layout(location = 0) out vec4 color;

uniform vec4 u_color;

void main()
{
	color = u_color;
}
#endif