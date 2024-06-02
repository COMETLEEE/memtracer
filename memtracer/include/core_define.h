#pragma once
#include <concrt.h>
#include <thread>
#include <concurrent_queue.h>
#include <concurrent_unordered_map.h>
#include <functional>
#include <mutex>

#include <windows.h>
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")

namespace memtracer
{
	// TODO : can be set user side.
	constexpr unsigned int MAX_STACK_FRAME = 32;

	using AllocFunc = std::function<void* (size_t)>;

	using FreeFunc = std::function<void(void*)>;

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

#define DEFAULT_REPORT_PATH ".\\memtracer_report.txt"
