#pragma once
#include <csignal>
#include <source_location>
#pragma warning(push)
#pragma warning( disable : 4005 )
#define IS_DEBUG 1
#ifndef NDEBUG
#define IS_DEBUG 0	
#endif 
#pragma warning(pop)
#define _USE_DETAILED_FUNCTION_NAME_IN_SOURCE_LOCATION 1
#if defined(SIGTRAP)
#define GENERAL_BREAK() raise(SIGTRAP)
#else
#define GENERAL_BREAK() (void)0;\
if (IS_DEBUG) \
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
	{\
		std::source_location sloc = std::source_location::current();\
		__glClearErrors();\
		x;\
		ASSERT(!__glLogCall(#x, sloc.file_name(), sloc.line(), sloc.column(), sloc.function_name()))\
	}\

#define __glCallReturn(x, out) \
	{\
		std::source_location sloc = std::source_location::current();\
		__glClearErrors();\
		out = x;\
		ASSERT(!__glLogCall(#x, sloc.file_name(), sloc.line(), sloc.column(), sloc.function_name()))\
	}\

void __glClearErrors();
bool __glLogCall(const char* function, const char* file, int line, int column, const char* func);
