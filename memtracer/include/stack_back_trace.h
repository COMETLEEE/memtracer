#pragma once
#include "core_define.h"

namespace memtracer
{
	class StackBackTrace final
	{
	public:
		StackBackTrace();

		~StackBackTrace();

		DELETE_CLASS_COPY_MOVE(StackBackTrace)

		void* operator new(size_t size);

		void operator delete(void* p);

		void* operator new[](size_t size);

		void operator delete[](void* p);

		CallStackHash get_call_stack_hash() const;

		FrameCount get_frame_count() const;

		void* get_stack_frame(int index) const;

	private:
		void* stack_frames[MAX_STACK_FRAME];

		FrameCount frame_count_;

		CallStackHash call_stack_hash_;
	};
}
