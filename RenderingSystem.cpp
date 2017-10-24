#include "RenderingSystem.h"

#include <cassert>

#include "Renderer.h"

#include "MemoryAllocator.h"
#include "FileIO.h"

#include "ModelFormat.h"
#include "RenderingComponent.h"

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
		RenderingComponent* comps = RenderingComponent::GetAllComponents();
		uint32_t count = RenderingComponent::GetComponentCount();
		
		for (uint32_t i = 0; i < count; ++i)
		{
			//if (!comp)
			// TODO
		}

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

		if (header->NumMeshes == 0)
		{
			return ModelHandle();
		}

		model::ModelMesh* meshes = reinterpret_cast<model::ModelMesh*>(header + 1);
		
		uint32_t verticesCount = 0;
		uint32_t indicesCount = 0;

		for (uint32_t i = 0; i < header->NumMeshes; ++i)
		{
			// Record Start Index

			verticesCount += meshes[i].NumVertices;
			indicesCount += meshes[i].NumIndices;
		}

		uint32_t vertexBufferSize = verticesCount * header->CalculateVertexSize();
		uint32_t indexBufferSize = indicesCount * sizeof(uint32_t);

		uint8_t* vertices = reinterpret_cast<uint8_t*>(meshes + header->NumMeshes);
		uint8_t* indices = vertices + vertexBufferSize;

		// Upload Data

		return ModelHandle();
	}

	MaterialHandle RenderingSystem::CreateMaterial(MaterialType type)
	{
		return MaterialHandle();
	}

}
