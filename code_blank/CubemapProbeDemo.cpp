#include "CubemapProbeDemo.h"

#include "Utility.h"

using namespace tofu;

int32_t CubemapProbeDemo::Init()
{
	CHECKED(sceneMgr.Init());

	CHECKED(sceneMgr.LoadScene("assets/scenes/test.json"));

	{
		Material* skyboxMat = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeSkybox);
		TextureHandle tex = RenderingSystem::instance()->CreateTexture("assets/craterlake.texture");
		skyboxMat->SetTexture(tex);
		ghostPlayer = new Utility::GhostPlayer(math::float3(-5.0f, 8.0f, -5.0f), skyboxMat);
	}

	return kOK;
}

int32_t CubemapProbeDemo::Shutdown()
{
	delete ghostPlayer;

	return kOK;
}

int32_t CubemapProbeDemo::Update()
{
	InputSystem* input = InputSystem::instance();
	if (input->IsButtonDown(ButtonId::kKeyEscape))
	{
		Engine::instance()->Quit();
	}

	CHECKED(ghostPlayer->Update());

	return kOK;
}
