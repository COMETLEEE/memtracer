#include "memory_tracer.h"
#include "memory_tracer_allocation.h"
#include <cassert>

namespace memtracer
{
	void MemoryTracer::initialize()
	{
		std::call_once(init_flag_, &MemoryTracer::init_instance);
	}

	void MemoryTracer::finalize()
	{
		std::call_once(finalize_flag_, &MemoryTracer::finalize_instance);
	}

	void MemoryTracer::start()
	{
		assert(instance_ != nullptr);

		// TODO : start tracer thread.

		// be last action.
		instance_->isInTrace_ = true;
	}

	void MemoryTracer::stop()
	{
		assert(instance_ != nullptr);

		instance_->isInTrace_ = false;

		// TODO : Sleep or lock (for global new, delete)

		// TODO : stop tracer thread.
	}

	void MemoryTracer::set_report_path(std::string path)
	{
		assert(instance_ != nullptr);

		instance_->report_path_ = std::move(path);
	}

	void MemoryTracer::init_instance()
	{
		instance_ = static_cast<MemoryTracer*>(memtracer_allocation(sizeof(MemoryTracer)));

		HANDLE handle = GetCurrentProcess();

		assert(SymInitialize(handle, NULL, TRUE));
	}

	void MemoryTracer::finalize_instance()
	{
		// TODO : clean up logic.
		instance_->address_to_size_map_.clear();

		memtracer_free(instance_);
	}
}
