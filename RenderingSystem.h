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

	private:
		Renderer*	renderer;

		HandleAllocator<ModelHandle, MAX_MODELS>			modelHandleAlloc;
		HandleAllocator<MeshHandle, MAX_MESHES>				meshHandleAlloc;
		HandleAllocator<MaterialHandle, MAX_MATERIALS>		materialHandleAlloc;

		std::unordered_map<std::string, ModelHandle>		modelTable;
	};

}

