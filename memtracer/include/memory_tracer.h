#pragma once
#include "core_define.h"
#include "memory_tracer_allocator.h"

namespace memtracer
{
	class StackBackTrace;

	struct MemoryOperation;

	template <void*(*Alloc)(size_t) = malloc
		, void*(*ArrayAlloc)(size_t) = malloc
		, void(*Free)(void*) = free
		, void(*ArrayFree)(void*) = free>
	class MemoryTracer final
	{
	public:
#pragma region for_users
		static void start();

		static void stop();

		static void set_report_path(char* path);

		static MemoryTracer* get_instance();

		static void* add_allocation(size_t size);

		static void remove_allocation(void* block);
#pragma endregion

	private:
		MemoryTracer();

		~MemoryTracer();

		static void init_instance();

		static void finalize_instance();

		static std::once_flag init_flag_;

		static std::once_flag finalize_flag_;

		static MemoryTracer* instance_;

		void* operator new(size_t size);

		void operator delete(void* block);

		AllocFunc alloc_ = Alloc;

		AllocFunc array_alloc_ = ArrayAlloc;

		FreeFunc free_ = Free;

		FreeFunc array_free_ = ArrayFree;

		// INTERNAL
		char report_path_[MAX_PATH];

		std::atomic<bool> is_in_trace_;

		concurrency::concurrent_queue<MemoryOperation*
			, memtracer::MemoryTracerAllocator<const MemoryOperation*>> memory_operations_;

		std::thread tracer_thread_;

#pragma region only_write_in_tracer_thread
		concurrency::concurrent_unordered_map<void*, size_t, std::hash<void*>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<const void*, size_t>>>
			address_to_size_map_;

		concurrency::concurrent_unordered_map<void*, CallStackHash, std::hash<void*>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<const void*, CallStackHash>>>
			address_to_hash_map_;

		concurrency::concurrent_unordered_map<CallStackHash, class memtracer::StackBackTrace*, std::hash<CallStackHash>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<const CallStackHash, class memtracer::StackBackTrace*>>>
			hash_to_stack_back_trace_map_;

		concurrency::concurrent_unordered_map<CallStackHash, size_t, std::hash<CallStackHash>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<const CallStackHash, size_t>>>
			hash_to_memory_allocation_;

		size_t total_memory_allocation_;

		size_t total_memory_allocation_count_;
#pragma endregion
	};

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::start()
	{
		assert(instance_ != nullptr);

		// TODO : start tracer thread.

		// be last action.
		instance_->is_in_trace_= true;
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::stop()
	{
		assert(instance_ != nullptr);

		instance_->is_in_trace_ = false;

		// TODO : Sleep or lock (for global new, delete)

		// TODO : stop tracer thread.
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::set_report_path(char* path)
	{
		assert(instance_ != nullptr);

		instance_.report_path_ = path;
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>* MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::get_instance()
	{
		std::call_once(init_flag_, &MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::init_instance);

		return instance_;
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void* MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::add_allocation(size_t size)
	{
		assert(instance_ != nullptr);

		void* block = Alloc(size);

		if (instance_->is_in_trace_ == true)
		{
			
		}

		return block;
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::remove_allocation(void* block)
	{
		assert(instance_ != nullptr);

		if (instance_->is_in_trace_ == true)
		{

		}

		Free(block);
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::MemoryTracer() :
		total_memory_allocation_(0)
		, total_memory_allocation_count_(0)
		, report_path_(DEFAULT_REPORT_PATH)
		, memory_operations_()
		, tracer_thread_()
	{
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::~MemoryTracer()
	{
		// TODO : clean up logic.
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::init_instance()
	{
		instance_ = new MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>();

		HANDLE process_handle = GetCurrentProcess();

		SymInitialize(process_handle, NULL, TRUE);
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::finalize_instance()
	{
		HANDLE process_handle = GetCurrentProcess();

		SymCleanup(process_handle);

		delete instance_;
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void* MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::operator new(size_t size)
	{
		return memtracer_allocation(sizeof(MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>));
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::operator delete(void* block)
	{
		memtracer_free(instance_);
	}
}
