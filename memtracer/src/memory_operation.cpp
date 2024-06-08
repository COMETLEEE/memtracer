#include "memory_operation.h"
#include "memory_tracer_allocation.h"

namespace memtracer
{
	MemoryOperation::MemoryOperation(EOperationType operation, void* address, size_t size, StackBackTrace* stack_back_trace) :
		operation_(operation)
		, address_(address)
		, size_(size)
		, stack_back_trace_(stack_back_trace)
	{

	}

	void* MemoryOperation::operator new(size_t size)
	{
		return memtracer_alloc(size);
	}

	void MemoryOperation::operator delete(void* block)
	{
		memtracer_free(block);
	}
}
