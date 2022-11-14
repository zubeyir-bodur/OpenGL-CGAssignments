#pragma once
#include <csignal>

#define IS_DEBUG 1
#ifndef NDEBUG
#define IS_DEBUG 0	
#endif 

#if defined(SIGTRAP)
#define GENERAL_BREAK() raise(SIGTRAP)
#else
#define GENERAL_BREAK() if (IS_DEBUG) \
 { \
	 __debugbreak();\
 } \
else \
{ \
	raise(3);\
}
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
