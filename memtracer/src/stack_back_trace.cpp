#include "stack_back_trace.h"

#include <cassert>

#include "memory_tracer_allocation.h"

namespace memtracer
{
	StackBackTrace::StackBackTrace()
	{
		frame_count_ = CaptureStackBackTrace(FRAMES_TO_SKIP, MAX_STACK_FRAMES, stack_frames, &call_stack_hash_);
	}

	StackBackTrace::~StackBackTrace()
	{
		ZeroMemory(stack_frames, sizeof(size_t) * MAX_STACK_FRAMES);

		frame_count_ = 0;

		call_stack_hash_ = 0;
	}

	void* StackBackTrace::operator new(size_t size)
	{
		return memtracer_alloc(size);
	}

	void StackBackTrace::operator delete(void* p)
	{
		memtracer_free(p);
	}

	void* StackBackTrace::operator new [](size_t size)
	{
		return memtracer_alloc(size);
	}

	void StackBackTrace::operator delete [](void* p)
	{
		memtracer_free(p);
	}

	CallStackHash StackBackTrace::get_call_stack_hash() const
	{
		return call_stack_hash_;
	}

	FrameCount StackBackTrace::get_frame_count() const
	{
		return frame_count_;
	}

	void* StackBackTrace::get_stack_frame(FrameCount index) const
	{
		assert(index < frame_count_);

		return stack_frames[index];
	}
}
