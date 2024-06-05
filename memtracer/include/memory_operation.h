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

	struct MemoryOperation final
	{
		MemoryOperation(EOperationType operation, void* address, size_t size, memtracer::StackBackTrace* stack_back_trace);

		EOperationType operation_;

		void* address_;

		size_t size_;

		memtracer::StackBackTrace* stack_back_trace_;
	};
}
