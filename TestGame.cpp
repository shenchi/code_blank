#include "Engine.h"
#include "TestGame.h"

#include "Entity.h"
#include "RenderingComponent.h"

using namespace tofu;

int32_t TestGame::Init()
{
	Entity e = Entity::Create();
	Component<RenderingComponent> r = e.AddComponent<RenderingComponent>();
	
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
