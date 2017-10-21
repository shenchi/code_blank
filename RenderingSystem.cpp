#include "RenderingSystem.h"

#include <cassert>

#include "Renderer.h"
#include "ModelLoader.h"

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

	MeshHandle RenderingSystem::CreateMesh(const char * filename)
	{
		//model::LoadModelFile()
		return MeshHandle();
	}

}
