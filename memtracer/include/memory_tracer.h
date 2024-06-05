#pragma once
#include "core_define.h"
#include "memory_operation.h"
#include "memory_tracer_allocator.h"
#include "stack_back_trace.h"

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

		static void take_snapshot();

		static void stop();

		static void set_report_path(char* path);

		static MemoryTracer* get_instance();

		static void* add_allocation(size_t size);

		static void remove_allocation(void* block);
#pragma endregion

		void* operator new[](size_t size) = delete;

		void operator delete[](void* block) = delete;

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

		void thread_update();

		void apply_allocation(MemoryOperation* memory_operation);

		void apply_free(MemoryOperation* memory_operation);

		// only function that initialize symbol and use it.
		void make_snapshot();

		AllocFunc alloc_ = Alloc;

		AllocFunc array_alloc_ = ArrayAlloc;

		FreeFunc free_ = Free;

		FreeFunc array_free_ = ArrayFree;

#pragma region internal
		char report_path_[MAX_PATH];

		std::atomic<bool> is_in_trace_;

		concurrency::concurrent_queue<MemoryOperation*
			, memtracer::MemoryTracerAllocator<const MemoryOperation*>> memory_operations_;

		std::thread tracer_thread_;

		std::recursive_mutex memory_information_mutex_;

#pragma region only_write_in_tracer_thread
		std::unordered_map<void*, size_t, std::hash<void*>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<const void*, size_t>>>
			address_to_size_map_;

		std::unordered_map<void*, CallStackHash, std::hash<void*>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<const void*, CallStackHash>>>
			address_to_hash_map_;

		std::unordered_map<CallStackHash, class memtracer::StackBackTrace*, std::hash<CallStackHash>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<const CallStackHash, class memtracer::StackBackTrace*>>>
			hash_to_stack_back_trace_map_;

		std::unordered_map<CallStackHash, size_t, std::hash<CallStackHash>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<const CallStackHash, size_t>>>
			hash_to_memory_allocation_map_;

		std::unordered_map<CallStackHash, size_t, std::hash<CallStackHash>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<const CallStackHash, size_t>>>
			hash_to_memory_allocation_count_map_;

		size_t total_memory_allocation_;

		size_t total_memory_allocation_count_;
#pragma endregion
#pragma endregion
	};

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::start()
	{
		assert(instance_ != nullptr);

		instance_->tracer_thread_ = std::thread(thread_update);

		instance_->is_in_trace_= true;
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::take_snapshot()
	{
		if (instance_->is_in_trace_ == true)
		{
			MemoryOperation* memory_operation = new MemoryOperation(EOperationType::Snapshot, nullptr, 0ull, nullptr);

			instance_->memory_operations_.push(memory_operation);
		}
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::stop()
	{
		assert(instance_ != nullptr);

		instance_->is_in_trace_ = false;

		// TODO : Wait last memory allocation and free
		Sleep(2000);

		// TODO : stop tracer thread.
		if (instance_->tracer_thread_.joinable() == true)
		{
			instance_->is_stop_requested_ = true;

			instance_->tracer_thread_.join();
		}
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
			StackBackTrace* stack_back_trace = new StackBackTrace();

			MemoryOperation* memory_operation = new MemoryOperation(EOperationType::Allocate, block, size, new StackBackTrace());

			instance_->memory_operations_.push(memory_operation);
		}

		return block;
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::remove_allocation(void* block)
	{
		assert(instance_ != nullptr);

		if (instance_->is_in_trace_ == true &&
			instance_->address_to_size_map_[block] != 0)
		{
			size_t size = instance_->address_to_size_map_[block];

			MemoryOperation* memory_operation = new MemoryOperation(EOperationType::Free, block, size, nullptr);

			instance_->memory_operations_.push(memory_operation);
		}

		Free(block);
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::MemoryTracer() :
		total_memory_allocation_(0)
		, total_memory_allocation_count_(0)
		, report_path_(DEFAULT_REPORT_PATH)
		, is_in_trace_(false)
		, memory_operations_()
		, tracer_thread_()
		, address_to_size_map_()
		, address_to_hash_map_()
		, hash_to_stack_back_trace_map_()
		, hash_to_memory_allocation_map_()
		, hash_to_memory_allocation_count_map_()
		
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

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::thread_update()
	{
		while (true)
		{
			MemoryOperation* memory_operation = nullptr;

			if (memory_operations_.try_pop(memory_operation) == true)
			{
				if (memory_operation->operation_ == EOperationType::Allocate)
				{
					apply_allocation(memory_operation);
				}
				else if (memory_operation->operation_ == EOperationType::Free)
				{
					apply_free(memory_operation);
				}
				else if (memory_operation->operation_ == EOperationType::Snapshot)
				{
					make_snapshot();
				}
				else if (memory_operation->operation_ == EOperationType::Stop)
				{
					// TODO : stop thread.
					break;
				}
			}

			delete memory_operation;
		}
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::apply_allocation(MemoryOperation* memory_operation)
	{
		void* address = memory_operation->address_;

		size_t size = memory_operation->size_;

		StackBackTrace* stack_back_trace = memory_operation->stack_back_trace_;

		const CallStackHash hash = stack_back_trace->get_call_stack_hash();

		address_to_size_map_[address] = size;

		address_to_hash_map_[address] = hash;

		// manage 'StackBackTrace'
		if (hash_to_stack_back_trace_map_.find(hash) == hash_to_stack_back_trace_map_.end())
		{
			hash_to_stack_back_trace_map_[hash] = stack_back_trace;
		}
		else
		{
			delete stack_back_trace;
		}

		hash_to_memory_allocation_map_[hash] += size;

		hash_to_memory_allocation_count_map_[hash] += 1;

		total_memory_allocation_ += size;

		total_memory_allocation_count_ += 1;
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::apply_free(MemoryOperation* memory_operation)
	{
		void* address = memory_operation->address_;

		if (address_to_size_map_.find(address) == address_to_size_map_.end())
			return;

		size_t size = 

		address_to_size_map_[address] = 0;

		total_memory_allocation_ -= size;
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::make_snapshot()
	{
	}
}
