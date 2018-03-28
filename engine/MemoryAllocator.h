#pragma once

#include "Common.h"

namespace tofu
{
	// sets of allocator for different scenario
	enum AllocatorType
	{
		kAllocGlobal,
		kAllocLevel,
		kAllocFrame,
		kAllocFrameEnd = kAllocFrame + kFrameBufferCount - 1,
		kAllocGlobalVMem,
		kAllocLevelVMem,
		kAllocFrameVMem,
		kAllocFrameVMemEnd = kAllocFrameVMem + kFrameBufferCount - 1,
		kMaxMemoryAllocators,
	};

	// interface for native memory allocation API wraping
	struct NativeAllocator
	{
		virtual void* Allocate(size_t size, size_t alignment) = 0;
		virtual int32_t Deallocate(void* addr, size_t size) = 0;
	};

	// global memory allocator set
	// (standard library containters still using system malloc/free)
	class MemoryAllocator
	{
	public:
		static MemoryAllocator Allocators[kMaxMemoryAllocators];

		// default native allocation API
		static NativeAllocator* DefaultNativeAllocator;

		// allocate and new
		template<typename T>
		static T* Alloc(uint32_t allocNo, size_t alignment = 4u)
		{
			void* ptr = Allocators[allocNo].Allocate(sizeof(T), alignment);
			return new(ptr) T();
		}

		TF_INLINE static void* Alloc(size_t size, uint32_t allocNo, size_t alignment = 4u)
		{
			return Allocators[allocNo].Allocate(size, alignment);
		}

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
		size_t				currentSize;
	};
}
