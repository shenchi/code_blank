#pragma once

#include "Common.h"
#include "Module.h"

#include "HandleAllocator.h"

#include <unordered_map>
#include <string>

namespace tofu
{
	class Renderer;

	HANDLE_DECL(Mesh);
	HANDLE_DECL(Model);
	HANDLE_DECL(Material);
	HANDLE_DECL(VertexBuffer);
	HANDLE_DECL(IndexBuffer);

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
		HandleAllocator<VertexBufferHandle, MAX_VERTEX_BUFFERS> vbHandleAlloc;
		HandleAllocator<IndexBufferHandle, MAX_INDEX_BUFFERS>	ibHandleAlloc;

		std::unordered_map<std::string, ModelHandle>		modelTable;
	};

}

