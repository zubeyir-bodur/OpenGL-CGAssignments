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

void main()
{
	color = vec4(0.1, 0.0, 0.8, 1.0);
}
#endif