#include "Engine.h"
#include "TestGame.h"

#include "RenderingSystem.h"

using namespace tofu;

int32_t TestGame::Init()
{
	{
		Entity e = Entity::Create();

		t1 = e.AddComponent<TransformComponent>();

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		ModelHandle model = RenderingSystem::instance()->CreateModel("assets/cube.model");
		MaterialHandle material = RenderingSystem::instance()->CreateMaterial(MaterialType::TestMaterial);

		r->SetMaterial(material);
		r->SetModel(model);
	}

	{
		Entity e = Entity::Create();

		TransformComponent t = e.AddComponent<TransformComponent>();

		CameraComponent camera = e.AddComponent<CameraComponent>();
		
		camera->SetAspect(800.0f / 600.0f);
		t->SetLocalPosition(math::float3{ 0, 0, -2 });
	}

	return TF_OK;
}

int32_t TestGame::Shutdown()
{
	return TF_OK;
}

int32_t TestGame::Update()
{
	static float time = 0.0f;
	time += Time::DeltaTime;
	t1->SetLocalPosition(math::float3{ sinf(time), 0, 0 });
	return TF_OK;
}
