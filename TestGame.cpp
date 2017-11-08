#include "Engine.h"
#include "TestGame.h"

#include "RenderingSystem.h"
#include "InputSystem.h"

using namespace tofu;

int32_t TestGame::Init()
{
	{
		Entity e = Entity::Create();

		t1 = e.AddComponent<TransformComponent>();

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		ModelHandle model = RenderingSystem::instance()->CreateModel("assets/cube.model");

		Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::OpaqueMaterial);
		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.DDS");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/stone_wall_normalmap.DDS");

		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);

		r->SetMaterial(material);
		r->SetModel(model);
	}

	{
		Entity e = Entity::Create();

		TransformComponent t = e.AddComponent<TransformComponent>();

		CameraComponent camera = e.AddComponent<CameraComponent>();
		
		camera->SetAspect(800.0f / 600.0f);
		t->SetLocalPosition(math::float3{ 0, 0, -2 });

		Material* skyboxMat = RenderingSystem::instance()->CreateMaterial(MaterialType::SkyboxMaterial);
		TextureHandle tex = RenderingSystem::instance()->CreateTexture("assets/craterlake.dds");
		skyboxMat->SetTexture(tex);

		camera->SetSkybox(skyboxMat);
	}

	return TF_OK;
}

int32_t TestGame::Shutdown()
{
	return TF_OK;
}

int32_t TestGame::Update()
{
	t1->SetLocalPosition(math::float3{ sinf(Time::TotalTime), 0, 0 });

	if (InputSystem::instance()->IsButtonDown(ButtonId::TF_KEY_Escape))
	{
		Engine::instance()->Quit();
	}

	return TF_OK;
}
