#include "RenderingSystem.h"

#include <cassert>

#include "Renderer.h"

#include "MemoryAllocator.h"
#include "FileIO.h"

namespace tofu
{

	SINGLETON_IMPL(RenderingSystem);

	RenderingSystem::RenderingSystem()
		:
		renderer(nullptr)
	{
		assert(nullptr == _instance);
		_instance = this;

		renderer = Renderer::CreateRenderer();
	}

	RenderingSystem::~RenderingSystem()
	{
		delete renderer;
	}

	int32_t RenderingSystem::Init()
	{
		assert(TF_OK == MemoryAllocator::Allocators[ALLOC_DEFAULT].Init(128 * 1024 * 1024, 2 * 1024 * 1024));
		return TF_OK;
	}

	int32_t RenderingSystem::Shutdown()
	{
		assert(TF_OK == MemoryAllocator::Allocators[ALLOC_DEFAULT].Shutdown());
		return TF_OK;
	}

	int32_t RenderingSystem::Update()
	{
		return TF_OK;
	}

	MeshHandle RenderingSystem::CreateMesh(const char * filename)
	{
		void* data = nullptr;
		size_t size = 0u;
		int32_t err = FileIO::ReadFile(filename, &data, &size, 4, ALLOC_DEFAULT);
		if (TF_OK != err)
		{

		}


		return MeshHandle();
	}

}
