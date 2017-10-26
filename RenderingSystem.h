#pragma once

#include "Common.h"
#include "Module.h"

#include "Renderer.h"

#include "HandleAllocator.h"

#include <unordered_map>
#include <string>

namespace tofu
{
	HANDLE_DECL(Mesh);
	HANDLE_DECL(Model);
	HANDLE_DECL(Material);


	enum class MaterialType
	{
		TestMaterial,
		OpaqueMaterial,
		MaxMaterialTypes
	};

	class RenderingSystem : public Module
	{
		SINGLETON_DECL(RenderingSystem)

	public:
		RenderingSystem();
		~RenderingSystem();

	public:
		int32_t Init() override;
		int32_t Shutdown() override;

		int32_t Update() override;

		ModelHandle CreateModel(const char* filename);

		MaterialHandle CreateMaterial(MaterialType type);

	private:
		Renderer*	renderer;

		HandleAllocator<ModelHandle, MAX_MODELS>			modelHandleAlloc;
		HandleAllocator<MeshHandle, MAX_MESHES>				meshHandleAlloc;
		HandleAllocator<MaterialHandle, MAX_MATERIALS>		materialHandleAlloc;
		HandleAllocator<BufferHandle, MAX_BUFFERS>			bufferHandleAlloc;

		std::unordered_map<std::string, ModelHandle>		modelTable;

		size_t		frameNo;
	};

}

