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

	MemoryAllocator MemoryAllocator::Allocators[kMaxMemoryAllocators];
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
			return kErrUnknown;
		}

		memoryBase = ptr;
		memorySize = size;
		currentSize = 0u;

		this->nativeAlloc = nativeAlloc;

		return kOK;
	}

	int32_t MemoryAllocator::Shutdown()
	{
		int32_t err = kOK;

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

		return kOK;
	}

	void * MemoryAllocator::Allocate(size_t size, size_t alignment)
	{
		assert(nullptr != memoryBase);

		size_t offset = align_to(currentSize, alignment);

		if (offset + size > memorySize)
		{
			return nullptr;
		}

		currentSize = offset + size;

		return reinterpret_cast<uint8_t*>(memoryBase) + offset;
	}

}
