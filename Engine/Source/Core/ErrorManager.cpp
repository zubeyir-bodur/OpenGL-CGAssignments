#include "Core/ErrorManager.h"
#include <glew.h>
#include "magic_enum/magic_enum.hpp"
#include <iostream>
#include <cassert>

enum class GLErrorEnum
{
	NoError = GL_NO_ERROR,
	InvalidEnum = GL_INVALID_ENUM,
	InvalidValue = GL_INVALID_VALUE,
	InvalidOperation = GL_INVALID_OPERATION,
	StackOverflow = GL_STACK_OVERFLOW,
	StackUnderflow = GL_STACK_UNDERFLOW,
	OutOfMemory = GL_OUT_OF_MEMORY,
	InvalidFrameBufferOperation = GL_INVALID_FRAMEBUFFER_OPERATION
};

const char* to_string(GLenum gl_enum)
{
	return magic_enum::enum_name<GLErrorEnum>((GLErrorEnum)gl_enum).data();
}

void __glClearErrors()
{
	GLenum error = glGetError();
	while (error != GL_NO_ERROR)
	{
		std::cout << "Clearing [OpenGL Error] : " << to_string(error) << std::endl;
		error = glGetError();
	}
}

bool __glLogCall(const char* function, const char* file, int line)
{
	bool has_error = false;
	GLenum error = glGetError();
	while (error != GL_NO_ERROR)
	{
		std::cout << "[OpenGL Error]  (" << file << "): " << function << ":" << line << " = " << to_string(error) << std::endl;
		has_error = true;
		error = glGetError();
	}
	if (has_error)
	{
		std::cout << std::endl;
	}
	return has_error;
}