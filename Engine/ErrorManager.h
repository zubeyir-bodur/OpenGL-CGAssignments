#pragma once
#include <csignal>

#if defined(SIGTRAP)
#define GENERAL_BREAK() raise(SIGTRAP)
#else
#define GENERAL_BREAK() __debugbreak()
#endif

#define ASSERT(x) if (!(x)) \
{\
	GENERAL_BREAK();\
}\

#define __glCallVoid(x) \
	__glClearErrors();\
	x;\
	ASSERT(!__glLogCall(#x, __FILE__, __LINE__))

#define __glCallReturn(x, out) \
	__glClearErrors();\
	out = x;\
	ASSERT(!__glLogCall(#x, __FILE__, __LINE__))

void __glClearErrors();
bool __glLogCall(const char* function, const char* file, int line);
