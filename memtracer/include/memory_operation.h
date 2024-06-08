#pragma once
#include <cstddef>

namespace memtracer
{
	class StackBackTrace;

	enum class EOperationType : unsigned char
	{
		Allocate,
		Free,
		Snapshot,
		Stop
	};

	struct MemoryOperation
	{
		MemoryOperation(EOperationType operation, void* address, size_t size, class memtracer::StackBackTrace* stack_back_trace);

		EOperationType operation_;

		void* address_;

		size_t size_;

		class memtracer::StackBackTrace* stack_back_trace_;

		void* operator new(size_t size);

		void operator delete(void* block);
	};
}
