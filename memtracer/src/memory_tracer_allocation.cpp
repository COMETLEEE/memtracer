#include "memory_tracer_allocation.h"
#include <cstdlib>

namespace memtracer
{
	void* memtracer_alloc(size_t size)
	{
		return malloc(size);
	}

	void memtracer_free(void* p)
	{
		free(p);
	}
}
