#include "RenderingSystem.h"

#include <cassert>

#include "Renderer.h"

#include "MemoryAllocator.h"
#include "FileIO.h"

#include "ModelFormat.h"

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

	ModelHandle RenderingSystem::CreateModel(const char* filename)
	{
		std::string strFilename(filename);

		{
			auto iter = modelTable.find(strFilename);
			if (iter != modelTable.end())
			{
				return iter->second;
			}
		}

		uint8_t* data = nullptr;
		size_t size = 0u;
		int32_t err = FileIO::ReadFile(filename, reinterpret_cast<void**>(&data), &size, 4, ALLOC_DEFAULT);
		if (TF_OK != err)
		{
			return ModelHandle();
		}

		model::ModelHeader* header = reinterpret_cast<model::ModelHeader*>(data);

		assert(header->Magic == model::MODEL_FILE_MAGIC);
		assert(header->StructOfArray == 0);
		assert(header->HasIndices == 1);
		assert(header->HasTangent == 1);
		assert(header->NumTexcoordChannels == 1);


		return ModelHandle();
	}

}
