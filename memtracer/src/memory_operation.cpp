#include "memory_operation.h"

namespace memtracer
{
	MemoryOperation::MemoryOperation(EOperationType operation, void* address, size_t size, StackBackTrace* stack_back_trace) :
		operation_(operation)
		, address_(address)
		, size_(size)
		, stack_back_trace_(stack_back_trace)
	{

	}
}