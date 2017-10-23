#include "MemoryAllocator.h"

#include <cassert>
#ifdef _MSC_VER
#include <malloc.h>
#endif

namespace tofu
{

	MemoryAllocator MemoryAllocator::Allocators[MAX_MEMORY_ALLOCATOR];

	int32_t MemoryAllocator::Init(size_t size, size_t alignment, NativeAllocator * nativeAlloc)
	{
		assert(size != 0u);

		void* ptr = nullptr;
		if (nullptr != nativeAlloc)
		{
			ptr = nativeAlloc->Allocate(size, alignment);
		}
#ifdef _MSC_VER
		else
		{
			ptr = _aligned_malloc(size, alignment);
		}
#endif

		if (nullptr == ptr)
		{
			return TF_UNKNOWN_ERR;
		}

		memoryBase = ptr;
		memorySize = size;

		this->nativeAlloc = nativeAlloc;

		return TF_OK;
	}

	int32_t MemoryAllocator::Shutdown()
	{
		int32_t err = TF_OK;

		if (nullptr != memoryBase)
		{
			if (nullptr != nativeAlloc)
			{
				err = nativeAlloc->Deallocate(memoryBase, memorySize);
			}
#ifdef _MSC_VER
			else
			{
				_aligned_free(memoryBase);
			}
#endif
		}

		nativeAlloc = nullptr;
		memoryBase = nullptr;
		memorySize = 0u;

		return err;
	}

	int32_t MemoryAllocator::Reset()
	{
		assert(nullptr != memoryBase);
		return int32_t();
	}

	void * MemoryAllocator::Allocate(size_t size, size_t alignment)
	{
		assert(nullptr != memoryBase);
		return nullptr;
	}

}
