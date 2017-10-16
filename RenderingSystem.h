#pragma once

#include "Common.h"
#include "Module.h"

#include "Renderer.h"

namespace tofu
{
	
	class RenderingSystem : public Module
	{
		SINGLETON_DECL(RenderingSystem)

	public:
		RenderingSystem(RendererType type);
		~RenderingSystem();

	public:
		int32_t Init() override;
		int32_t Shutdown() override;

		int32_t Update() override;

	private:
		Renderer*	renderer;
	};

}

