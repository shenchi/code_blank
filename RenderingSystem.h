#pragma once

#include "Common.h"
#include "Module.h"

namespace tofu
{
	class Renderer;

	HANDLE_DECL(Mesh);
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

		MeshHandle CreateMesh(const char* filename);
		//MeshHandle CreateMesh()

	private:
		Renderer*	renderer;
	};

}

