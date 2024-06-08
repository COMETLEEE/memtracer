#pragma once
#include <concrt.h>
#include <cassert>
#include <thread>
#include <concurrent_queue.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <algorithm>
#include <iostream>

#include <windows.h>
#include <tchar.h>
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")

namespace memtracer
{
	// TODO : can be set user side.
	constexpr unsigned int MAX_STACK_FRAMES = 32;

	constexpr unsigned int FRAMES_TO_SKIP = 2;

	using AllocFunc = std::function<void* (size_t)>;

	using FreeFunc = std::function<void(void*)>;

	using FrameCount = WORD;

	using CallStackHash = DWORD;
}

#define DELETE_CLASS_COPY(Class)				\
	Class(const Class&) = delete;				\
	void operator=(const Class&) = delete;		\

#define DELETE_CLASS_MOVE(Class)				\
	Class(const Class&&) = delete;				\
	void operator=(const Class&&) = delete;		\

#define DELETE_CLASS_COPY_MOVE(Class)			\
	DELETE_CLASS_COPY(Class)					\
	DELETE_CLASS_MOVE(Class)					\

#define DEFAULT_REPORT_PATH TEXT(".\\MemoryTracer_Report")

#ifdef _UNICODE
	using tstring = std::wstring;
	using TSYMBOL_INFO = SYMBOL_INFOW;
	using TIMAGEHLP_LINE64 = IMAGEHLP_LINEW64;

#	define stprintf_s swprintf_s
#	define TSymFromAddr SymFromAddrW
#	define TSymGetLineFromAddr64 SymGetLineFromAddrW64
#else // _UNICODE
	using tstring = std::string;
	using TSYMBOL_INFO = SYMBOL_INFO;
	using TIMAGEHLP_LINE64 = IMAGEHLP_LINE64

#	define stprintf sprintf_s
#	define TSymFromAddr SymFromAddr
#	define TSymGetLineFromAddr64 SymGetLineFromAddr64
#endif // _UNICODE