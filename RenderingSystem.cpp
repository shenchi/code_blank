#include "RenderingSystem.h"

#include <cassert>

namespace tofu
{

	SINGLETON_IMPL(RenderingSystem);

	RenderingSystem::RenderingSystem(RendererType type)
		:
		renderer(nullptr)
	{
		assert(nullptr == _instance);
		_instance = this;

		renderer = Renderer::CreateRenderer(type);
	}

	RenderingSystem::~RenderingSystem()
	{
		delete renderer;
	}

	int32_t RenderingSystem::Init()
	{
		return TF_OK;
	}

	int32_t RenderingSystem::Shutdown()
	{
		return TF_OK;
	}

	int32_t RenderingSystem::Update()
	{
		return TF_OK;
	}

}
