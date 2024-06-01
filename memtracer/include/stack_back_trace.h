#pragma once
#include "call_stack_hash.h"
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

		void* operator new[](size_t size) = delete;

		void operator delete[](void* p) = delete;

	private:
		void* stack_frames[MAX_STACK_FRAME];

		unsigned int frame_count_;

		CallStackHash call_stack_hash_;
	};
}
