#pragma once
#include <cstddef>

namespace memtracer
{
	class StackBackTrace;

	enum class EOperationType : unsigned char
	{
		None,
		Allocate,
		Free,
		Snapshot,
		Stop
	};

	class IMemoryOperation
	{
	protected:
		IMemoryOperation(EOperationType type);

	public:
		EOperationType operation_type_;

		void* operator new(size_t size);

		void operator delete(void* block);
	};

	class AllocateOperation : public IMemoryOperation
	{
	public:
		AllocateOperation(void* address, size_t size, class memtracer::StackBackTrace* stack_back_trace);

		void* address_;

		size_t size_;

		class memtracer::StackBackTrace* stack_back_trace_;
	};

	class FreeOperation : public IMemoryOperation
	{
	public:
		FreeOperation(void* address);

		void* address_;
	};

	class SnapshotOperation : public IMemoryOperation
	{
	public:
		SnapshotOperation();
	};

	class StopOperation : public IMemoryOperation
	{
	public:
		StopOperation();
	};
}
