#pragma once
#include "memory_tracer_allocation.h"

namespace memtracer
{
	template <typename T>
	class MemoryTracerAllocator final
	{
        typedef T value_type;

        MemoryTracerAllocator() noexcept {}

        template<typename U>
        MemoryTracerAllocator(const MemoryTracerAllocator<U>&) noexcept {}

        static T* allocate(size_t n)
		{
            return static_cast<T*>(memtracer_allocation(n * sizeof(T)));
        }

        static void deallocate(T* p, size_t n) noexcept
		{
            memtracer_free(p);
        }
    };
}
