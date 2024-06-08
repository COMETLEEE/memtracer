#include "memory_operation.h"
#include "memory_tracer_allocation.h"

namespace memtracer
{
	IMemoryOperation::IMemoryOperation(EOperationType operation_type) :
		operation_type_(operation_type)
	{

	}

	void* IMemoryOperation::operator new(size_t size)
	{
		return memtracer_alloc(size);
	}

	void IMemoryOperation::operator delete(void* block)
	{
		memtracer_free(block);
	}

	AllocateOperation::AllocateOperation(void* address, size_t size, class memtracer::StackBackTrace* stack_back_trace) :
		IMemoryOperation(EOperationType::Allocate)
		, address_(address)
		, size_(size)
		, stack_back_trace_(stack_back_trace)
	{
	}

	FreeOperation::FreeOperation(void* address) :
		IMemoryOperation(EOperationType::Free)
		, address_(address)
	{
	}

	SnapshotOperation::SnapshotOperation() :
		IMemoryOperation(EOperationType::Snapshot)
	{
	}

	StopOperation::StopOperation() :
		IMemoryOperation(EOperationType::Stop)
	{
	}
}
