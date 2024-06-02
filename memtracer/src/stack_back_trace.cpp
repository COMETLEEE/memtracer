#include "stack_back_trace.h"
#include "memory_tracer_allocation.h"

namespace memtracer
{
	StackBackTrace::StackBackTrace()
	{
		frame_count_ = CaptureStackBackTrace(0, MAX_STACK_FRAME, stack_frames, &call_stack_hash_);
	}

	StackBackTrace::~StackBackTrace()
	{
		ZeroMemory(stack_frames, sizeof(size_t) * MAX_STACK_FRAME);

		frame_count_ = 0;

		call_stack_hash_ = 0;
	}

	void* StackBackTrace::operator new(size_t size)
	{
		return memtracer_allocation(size);
	}

	void StackBackTrace::operator delete(void* p)
	{
		memtracer_free(p);
	}
}
