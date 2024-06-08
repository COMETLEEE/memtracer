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
		static MemoryTracer* get_instance();

		void start();

		void take_snapshot();

		void stop();

		void set_report_path(TCHAR* path);

		void* add_allocation(size_t size);

		void remove_allocation(void* block);
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
		TCHAR report_path[MAX_PATH];

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

		std::unordered_map<CallStackHash, memtracer::StackBackTrace*, std::hash<CallStackHash>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<const CallStackHash, memtracer::StackBackTrace*>>>
			hash_to_stack_back_trace_map_;

		std::unordered_map<CallStackHash, size_t, std::hash<CallStackHash>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<const CallStackHash, size_t>>>
			hash_to_memory_allocation_map_;

		std::unordered_map<CallStackHash, size_t, std::hash<CallStackHash>
			, std::equal_to<>, memtracer::MemoryTracerAllocator<std::pair<const CallStackHash, size_t>>>
			hash_to_memory_allocation_count_map_;

		size_t total_memory_allocation_;

		size_t total_memory_allocation_count_;

		size_t snapshot_index;
#pragma endregion
#pragma endregion
	};

	template <void* (*Alloc)(size_t), void* (*ArrayAlloc)(size_t), void(*Free)(void*), void(*ArrayFree)(void*) >
	MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>* MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::instance_ = nullptr;

	template <void* (*Alloc)(size_t), void* (*ArrayAlloc)(size_t), void(*Free)(void*), void(*ArrayFree)(void*) >
	std::once_flag MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::init_flag_;

	template <void* (*Alloc)(size_t), void* (*ArrayAlloc)(size_t), void(*Free)(void*), void(*ArrayFree)(void*) >
	std::once_flag MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::finalize_flag_;

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::start()
	{
		assert(instance_ != nullptr);

		instance_->tracer_thread_ = std::thread(&MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::thread_update, this);

		instance_->is_in_trace_ = true;
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::take_snapshot()
	{
		assert(instance_ != nullptr);

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

		if (instance_->tracer_thread_.joinable() == true)
		{
			memtracer::MemoryOperation* memory_operation = new MemoryOperation(EOperationType::Stop, nullptr, 0, nullptr);

			instance_->memory_operations_.push(memory_operation);

			instance_->tracer_thread_.join();

			instance_->is_in_trace_ = false;
		}
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::set_report_path(TCHAR* path)
	{
		assert(instance_ != nullptr);

		instance_.report_path = path;
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

		if (instance_->is_in_trace_ == true)
		{
			MemoryOperation* memory_operation = new MemoryOperation(EOperationType::Free, block, 0, nullptr);

			instance_->memory_operations_.push(memory_operation);
		}

		Free(block);
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::MemoryTracer() :
		total_memory_allocation_(0)
		, total_memory_allocation_count_(0)
		, report_path(DEFAULT_REPORT_PATH)
		, is_in_trace_(false)
		, memory_operations_()
		, tracer_thread_()
		, address_to_size_map_()
		, address_to_hash_map_()
		, hash_to_stack_back_trace_map_()
		, hash_to_memory_allocation_map_()
		, hash_to_memory_allocation_count_map_()
		, snapshot_index(0)
	{
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::~MemoryTracer()
	{

	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::init_instance()
	{
		instance_ = new MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>();

		HANDLE process_handle = GetCurrentProcess();

		if (SymInitialize(process_handle, NULL, TRUE) != TRUE)
		{
			std::cerr << "SymInitialize failed." << std::endl;
		}
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
		return memtracer_alloc(sizeof(MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>));
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
					delete memory_operation;

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

		// do not apply memory allocations in prev start trace.
		if (address_to_size_map_.find(address) == address_to_size_map_.end())
			return;

		size_t size = address_to_size_map_[address];

		CallStackHash hash = address_to_hash_map_[address];

		address_to_hash_map_.erase(address);

		address_to_size_map_.erase(address);

		hash_to_memory_allocation_count_map_[hash] -= 1;

		hash_to_memory_allocation_map_[hash] -= size;

		if (hash_to_memory_allocation_count_map_[hash] == 0)
		{
			hash_to_memory_allocation_map_.erase(hash);

			hash_to_memory_allocation_count_map_.erase(hash);

			hash_to_stack_back_trace_map_.erase(hash);
		}

		total_memory_allocation_ -= size;

		total_memory_allocation_count_ -= 1;
	}

	template <void*(* Alloc)(size_t), void*(* ArrayAlloc)(size_t), void(* Free)(void*), void(* ArrayFree)(void*)>
	void MemoryTracer<Alloc, ArrayAlloc, Free, ArrayFree>::make_snapshot()
	{
		HANDLE process_handle = GetCurrentProcess();

		tstring report;
		
		tstring report_content;

		report.clear();

		report_content.clear();

		constexpr size_t buffer_size = 1024ull;

		TCHAR buffer[buffer_size] = { 0 };

		std::vector<std::pair<size_t, tstring>> report_contents = std::vector<std::pair<size_t, tstring>>();

		for (auto& pair : hash_to_stack_back_trace_map_)
		{
			ZeroMemory(buffer, buffer_size * sizeof(TCHAR));

			report_content.clear();

			CallStackHash hash = pair.first;

			size_t total_memory_allocation = hash_to_memory_allocation_map_[hash];

			size_t total_memory_allocation_count = hash_to_memory_allocation_count_map_[hash];

			stprintf_s(buffer, buffer_size, TEXT("------- %.2f MB / %llu times -------\r\n")
				, static_cast<float>(total_memory_allocation) / 1024ull / 1024ull
				, total_memory_allocation_count);

			report_content += buffer;

			StackBackTrace* stack_back_trace = pair.second;

			constexpr size_t symbol_size = sizeof(TSYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR);

			BYTE symbol_buffer[symbol_size] = { 0 };

			TSYMBOL_INFO* symbol = reinterpret_cast<TSYMBOL_INFO*>(symbol_buffer);

			for (size_t i = stack_back_trace->get_frame_count() - 1 ; i != 0 ; --i)
			{
				ZeroMemory(buffer, buffer_size);

				ZeroMemory(symbol, symbol_size);

				symbol->SizeOfStruct = sizeof(TSYMBOL_INFO);

				symbol->MaxNameLen = MAX_SYM_NAME;

				if (TSymFromAddr(process_handle, reinterpret_cast<DWORD64>(stack_back_trace->get_stack_frame(i)), NULL, symbol) == TRUE)
				{
					TIMAGEHLP_LINE64 line_info;

					ZeroMemory(&line_info, sizeof(TIMAGEHLP_LINE64));

					line_info.SizeOfStruct = sizeof(TIMAGEHLP_LINE64);
					DWORD displacement = 0;

					if (TSymGetLineFromAddr64(process_handle, reinterpret_cast<DWORD64>(stack_back_trace->get_stack_frame(i)), &displacement, &line_info) == TRUE)
					{
						stprintf_s(buffer, buffer_size, TEXT("%p - %s : %s (%d)\r\n"), reinterpret_cast<void*>(symbol->Address), symbol->Name, line_info.FileName, line_info.LineNumber);
					}
					else
					{
						stprintf_s(buffer, buffer_size, TEXT("%p - %s : Failed to get file info.\r\n"), reinterpret_cast<void*>(symbol->Address), symbol->Name);
					}
				}
				else
				{
					stprintf_s(buffer, buffer_size, TEXT("%p : Failed to get symbol info.\r\n"), stack_back_trace->get_stack_frame(i));
				}

				report_content += buffer;
			}

			report_contents.push_back({ total_memory_allocation, report_content });
		}

		std::sort(report_contents.begin(), report_contents.end(), [](const std::pair<size_t, tstring>& first, const std::pair<size_t, tstring>& second)
			{
				return first.first > second.first;
			});

		for (auto& pair_content : report_contents)
		{
			const tstring& content = pair_content.second;

			report += content;
		}

		// don't have any memory allocations.
		if (report.empty() == true)
		{
			report = TEXT("Don't have any memory allocations.");
		}

		if (CreateDirectory(report_path, NULL) == TRUE ||
			GetLastError() == ERROR_ALREADY_EXISTS)
		{
			stprintf_s(buffer, buffer_size, TEXT("%s\\MemoryTracer_Report #%llu.txt"), report_path, snapshot_index++);

			HANDLE file_handle = CreateFile(
				buffer
				, GENERIC_WRITE
				, 0
				, NULL
				, CREATE_ALWAYS
				, FILE_ATTRIBUTE_NORMAL
				, NULL);

			if (file_handle != INVALID_HANDLE_VALUE)
			{
				DWORD bytesWritten = 0;

				DWORD targetBytes = report.length() * sizeof(TCHAR);

				if (WriteFile(file_handle, report.c_str(), targetBytes, &bytesWritten, NULL) == TRUE)
				{
					if (targetBytes != bytesWritten)
					{
						std::cerr << "Failed to write file all snapshot's contents." << std::endl;
					}
				}
				else
				{
					std::cerr << "Failed to write snapshot file." << std::endl;
				}
			}
			else
			{
				std::cerr << "Failed to create snapshot file." << std::endl;
			}

			CloseHandle(file_handle);
		}
		else
		{
			std::cerr << "Failed to create snapshot directory." << std::endl;
		}

		CloseHandle(process_handle);
	}
}
