#pragma once

#include "common.h"

#include "Renderer.h"

namespace tofu
{
	
	class RenderingSystem
	{
		SINGLETON_DECL(RenderingSystem)

	public:
		RenderingSystem(RendererType type);
		~RenderingSystem();

	public:
		int32_t Init();
		int32_t Shutdown();

	private:
		Renderer*	renderer;
	};

}

