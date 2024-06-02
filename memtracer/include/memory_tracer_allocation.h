#pragma once

namespace memtracer
{
	void* memtracer_alloc(size_t size);

	void memtracer_free(void* p);
}
