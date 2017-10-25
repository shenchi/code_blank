#pragma once

#include "Common.h"

namespace tofu
{
	enum AllocatorType
	{
		ALLOC_DEFAULT,
		ALLOC_LEVEL_BASED_MEM,
		ALLOC_FRAME_BASED_MEM,
		ALLOC_FRAME_BASED_MEM_END = ALLOC_FRAME_BASED_MEM + FRAME_BUFFER_COUNT - 1,
		ALOOC_LEVEL_BASED_VMEN,
		ALLOC_FRAME_BASED_VMEM,
		ALLOC_FRAME_BASED_VMEN_END = ALLOC_FRAME_BASED_VMEM + FRAME_BUFFER_COUNT - 1,
		MAX_MEMORY_ALLOCATOR,
	};

	//constexpr uint32_t MAX_MEMORY_ALLOCATOR = 16;

	struct NativeAllocator
	{
		virtual void* Allocate(size_t size, size_t alignment) = 0;
		virtual int32_t Deallocate(void* addr, size_t size) = 0;
	};

	class MemoryAllocator
	{
	public:
		static MemoryAllocator Allocators[MAX_MEMORY_ALLOCATOR];
		static NativeAllocator* DefaultNativeAllocator;

	private:
		MemoryAllocator() : nativeAlloc(nullptr), memoryBase(nullptr), memorySize(0u) {}
		//~MemoryAllocator();

	public:
		MemoryAllocator(const MemoryAllocator&) = delete;
		MemoryAllocator(MemoryAllocator&&) = delete;

		MemoryAllocator& operator = (const MemoryAllocator&) = delete;
		MemoryAllocator& operator = (MemoryAllocator&&) = delete;

	public:

		int32_t Init(size_t size, size_t alignment, NativeAllocator* nativeAlloc = nullptr);

		int32_t Shutdown();

		int32_t Reset();

		void* Allocate(size_t size, size_t alignment);

	private:
		NativeAllocator*	nativeAlloc;
		void*				memoryBase;
		size_t				memorySize;
	};
}
