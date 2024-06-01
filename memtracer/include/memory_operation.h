#pragma once
#include <cstddef>

namespace memtracer
{
	enum class EOperationType : unsigned char
	{
		Allocate,
		Free
	};

	struct MemoryOperation final
	{
		EOperationType operation_;

		void* address_;

		size_t size_;
	};
}
