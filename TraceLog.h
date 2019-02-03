#pragma once
#include<iostream>
#include<stdarg.h>
#include<string>
using namespace std;
#define __DEBUG__
static string GetFileName(const string& path)
{
	char ch = '/';
#ifdef _WIN32
	ch = '\\';
#endif
	size_t pos = path.rfind(ch);
	if (pos == string::npos)
		return path;
	else
		return path.substr(pos + 1);
}
//用于调试追溯的trace log
inline static void __trace_debug(const char* function,
	const char* filename, int line, const char* format, ...)
{
#ifdef __DEBUG__
	//输出调用函数的信息
	fprintf(stdout, "【%s:%d】%s", GetFileName(filename).c_str(), line, function);
	//输出用户打的trace信息
	va_list args;
	va_start (args, format);
	vfprintf(stdout, format, args);
	va_end(args);
#endif
}
#define __TRACE_DEBUG(...) \
__trace_debug(__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
