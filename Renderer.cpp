#include "Renderer.h"

#ifdef _MSC_VER
#include "RendererDX11.h"
#endif

#include "MemoryAllocator.h"

#include <assert.h>

namespace tofu
{

	Renderer* Renderer::CreateRenderer()
	{
#ifdef _MSC_VER
		return dx11::CreateRendererDX11();
#else
		return nullptr;
#endif
	}

	RendererCommandBuffer * RendererCommandBuffer::Create(uint32_t capacity, uint32_t allocNo)
	{
		MemoryAllocator& alloc = MemoryAllocator::Allocators[allocNo];

		void* ptr = alloc.Allocate(sizeof(RendererCommandBuffer), 4);
		assert(nullptr != ptr);

		RendererCommandBuffer* buf = reinterpret_cast<RendererCommandBuffer*>(ptr);

		ptr = alloc.Allocate(sizeof(uint32_t) * capacity, sizeof(uint32_t));
		assert(nullptr != ptr);
		buf->cmds = reinterpret_cast<uint32_t*>(ptr);

		ptr = alloc.Allocate(sizeof(void*) * capacity, sizeof(void*));
		assert(nullptr != ptr);
		buf->params = reinterpret_cast<void**>(ptr);

		buf->capacity = capacity;
		buf->size = 0;

		return buf;
	}

	void RendererCommandBuffer::Add(uint32_t cmd, void* param)
	{
		assert(size < capacity);

		cmds[size] = cmd;
		params[size] = param;

		size++;
	}

}