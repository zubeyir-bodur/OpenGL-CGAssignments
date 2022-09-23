#include "ErrorManager.h"
#include "glew.h"
#include "GL/GL.h"
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

static const char* GLErrorEnum_string[] = {
	"NoError",
	"InvalidEnum",
	"InvalidValue",
	"InvalidOperation",
	"InvalidFrameBufferOperation",
	"OutOfMemory",
	"StackUnderflow",
	"StackOverflow"
};

const char* to_string(GLErrorEnum error)
{
	if (error == (GLErrorEnum)0)
	{
		return GLErrorEnum_string[0];
	}
	else
	{
		unsigned int index = (unsigned int)error - 0x4ff;
		return GLErrorEnum_string[index];
	}
}

void __glClearErrors()
{
	GLErrorEnum error = (GLErrorEnum)glGetError();
	while (error != GLErrorEnum::NoError)
	{
		std::cout << "Clearing [OpenGL Error] : " << to_string(error) << std::endl;
		error = (GLErrorEnum)glGetError();
	}
}

bool __glLogCall(const char* function, const char* file, int line)
{
	bool has_error = false;
	GLErrorEnum error = (GLErrorEnum)glGetError();
	while (error != GLErrorEnum::NoError)
	{
		std::cout << "[OpenGL Error]  (" << file << "): " << function << ":" << line << " = " << to_string(error) << std::endl;
		has_error = true;
		error = (GLErrorEnum)glGetError();
	}
	if (has_error)
	{
		std::cout << std::endl;
	}
	return has_error;
}