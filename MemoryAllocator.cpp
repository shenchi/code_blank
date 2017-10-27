#include "MemoryAllocator.h"

#include <cassert>
#ifdef _MSC_VER
#include <malloc.h>
#endif

namespace
{
	constexpr size_t align_to(size_t address, size_t alignment)
	{
		return (address + (alignment - 1u)) & ~(alignment - 1u);
	}
}

namespace tofu
{

	MemoryAllocator MemoryAllocator::Allocators[MAX_MEMORY_ALLOCATOR];
	NativeAllocator* MemoryAllocator::DefaultNativeAllocator = nullptr;

	int32_t MemoryAllocator::Init(size_t size, size_t alignment, NativeAllocator * nativeAlloc)
	{
		assert(size != 0u);

		void* ptr = nullptr;
		if (nullptr != nativeAlloc)
		{
			ptr = nativeAlloc->Allocate(size, alignment);
		}
		else if (nullptr != DefaultNativeAllocator)
		{
			ptr = DefaultNativeAllocator->Allocate(size, alignment);
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

		currentSize = 0u;

		return TF_OK;
	}

	void * MemoryAllocator::Allocate(size_t size, size_t alignment)
	{
		assert(nullptr != memoryBase);

		size_t newSize = align_to(currentSize + size, alignment);

		if (newSize > memorySize)
		{
			return nullptr;
		}

		currentSize = newSize;

		return reinterpret_cast<uint8_t*>(memoryBase) + newSize;
	}

}
