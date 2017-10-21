#pragma once

#include "Common.h"

namespace tofu
{
	struct NativeAllocator
	{
		virtual void* Allocate(size_t size, size_t alignment) = 0;
		virtual int32_t Deallocate(void* addr, size_t size) = 0;
	};

	class MemoryAllocator
	{
	public:
		//enum 
	public:
		MemoryAllocator(NativeAllocator* _alloc) : alloc(_alloc) {}

		void* AllocateForFrame(size_t size, size_t alignment);

	private:
		NativeAllocator*	alloc;
	};
}
