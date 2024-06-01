#pragma once
#include "call_stack_hash.h"
#include "core_define.h"
#include "memory_tracer_allocator.h"

namespace memtracer
{
	class StackBackTrace;
	struct MemoryOperation;

	class MemoryTracer final
	{
	public:
#pragma region for_users
		static void initialize();

		static void finalize();

		static void start();

		static void stop();

		static void set_report_path(std::string path);
#pragma endregion

	private:
		static MemoryTracer* instance_;

		static void init_instance();

		static void finalize_instance();

		static std::once_flag init_flag_;

		static std::once_flag finalize_flag_;

		static constexpr std::function<void*(size_t)> alloc_;

		static constexpr std::function<void*(size_t)> array_alloc_;

		static constexpr std::function<void(void*)> free_;

		static constexpr std::function<void(void*)> array_free_;








		std::string report_path_;

		std::atomic<bool> isInTrace_;

		std::thread<void(void)> tracer_thread_;

		// set custom allocator
		concurrency::concurrent_queue<MemoryOperation*
			, memtracer::MemoryTracerAllocator<MemoryOperation*>> memory_operations_;

#pragma region only_write_in_tracer_thread
		std::unordered_map<void*, size_t, std::hash<void*>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<void*, size_t>>>
			address_to_size_map_;

		std::unordered_map<void*, memtracer::CallStackHash, std::hash<void*>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<void*, memtracer::CallStackHash>>>
			address_to_hash_map_;

		std::unordered_map<memtracer::CallStackHash, class memtracer::StackBackTrace*, std::hash<memtracer::CallStackHash>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<memtracer::CallStackHash, class memtracer::StackBackTrace*>>>
			hash_to_stack_back_trace_map_;

		std::unordered_map<memtracer::CallStackHash, size_t, std::hash<memtracer::CallStackHash>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<memtracer::CallStackHash, size_t>>>
			hash_to_memory_allocation_;

		size_t totalMemoryAllocation_;

		size_t totalMemoryAllocationCount_;
#pragma endregion

		friend MemoryTracer* addAllocation(void* p, size_t size);

		friend void removeAllocation(void* p);
	};
}
