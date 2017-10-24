#include "Engine.h"
#include "TestGame.h"

#include "Entity.h"
#include "TransformComponent.h"
#include "RenderingComponent.h"
#include "RenderingSystem.h"

using namespace tofu;

int32_t TestGame::Init()
{
	Entity e = Entity::Create();

	Component<TransformComponent> t = e.AddComponent<TransformComponent>();

	Component<RenderingComponent> r = e.AddComponent<RenderingComponent>();

	ModelHandle model = RenderingSystem::instance()->CreateModel("assets/torus.model");
	MaterialHandle material = RenderingSystem::instance()->CreateMaterial(MaterialType::TestMaterial);

	r->SetMaterial(material);
	r->SetModel(model);

	return TF_OK;
}

int32_t TestGame::Shutdown()
{
	return TF_OK;
}

int32_t TestGame::Update()
{
	return TF_OK;
}
