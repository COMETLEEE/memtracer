#pragma once

namespace memtracer
{
	void* memtracer_allocation(size_t size);

	void memtracer_free(void* p);
}
