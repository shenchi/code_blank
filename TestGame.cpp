#include "Engine.h"
#include "TestGame.h"

#include "RenderingSystem.h"
#include "InputSystem.h"

using namespace tofu;

int32_t TestGame::Init()
{
	{
		Entity e = Entity::Create();

		tCube = e.AddComponent<TransformComponent>();
		//tCube->SetLocalPosition(math::float3{ 1, 0, 0 });

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		Model* model = RenderingSystem::instance()->CreateModel("assets/cube.model");

		Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::OpaqueMaterial);
		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/stone_wall_normalmap.texture");

		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);

		r->SetMaterial(material);
		r->SetModel(model);
	}

	{
		Entity e = Entity::Create();

		tCamera = e.AddComponent<TransformComponent>();

		CameraComponent camera = e.AddComponent<CameraComponent>();
		
		camera->SetFOV(60.0f);
		tCamera->SetLocalPosition(math::float3{ 0, 0, -2 });

		Material* skyboxMat = RenderingSystem::instance()->CreateMaterial(MaterialType::SkyboxMaterial);
		TextureHandle tex = RenderingSystem::instance()->CreateTexture("assets/craterlake.texture");
		skyboxMat->SetTexture(tex);

		camera->SetSkybox(skyboxMat);
	}

	pitch = 0.0f;
	yaw = 0.0f;

	return TF_OK;
}

int32_t TestGame::Shutdown()
{
	return TF_OK;
}

int32_t TestGame::Update()
{
	InputSystem* input = InputSystem::instance();
	if (input->IsButtonDown(ButtonId::TF_KEY_Escape))
	{
		Engine::instance()->Quit();
	}

	constexpr float sensitive = 0.01f;

	if (input->IsGamepadConnected())
	{
		if (input->IsButtonDown(ButtonId::TF_GAMEPAD_FACE_RIGHT))
		{
			Engine::instance()->Quit();
		}

		float lt = input->GetLeftTrigger();
		float rt = input->GetRightTrigger();

		float up = rt - lt;
		
		tCamera->Translate(Time::DeltaTime * (
			tCamera->GetForwardVector() * -input->GetLeftStickY()
			+ tCamera->GetRightVector() * input->GetLeftStickX()
			+ tCamera->GetUpVector() * up
			));

		pitch += sensitive * input->GetRightStickY();
		yaw += sensitive * input->GetRightStickX();
	}

	pitch += sensitive * input->GetMouseDeltaY();
	yaw += sensitive * input->GetMouseDeltaX();

	tCamera->SetLocalRotation(math::quat(pitch, yaw, 0.0f));

	if (input->IsButtonDown(TF_KEY_W))
	{
		tCamera->Translate(tCamera->GetForwardVector() * Time::DeltaTime);
	}
	else if (input->IsButtonDown(TF_KEY_S))
	{
		tCamera->Translate(-tCamera->GetForwardVector() * Time::DeltaTime);
	}

	if (input->IsButtonDown(TF_KEY_D))
	{
		tCamera->Translate(tCamera->GetRightVector() * Time::DeltaTime);
	}
	else if (input->IsButtonDown(TF_KEY_A))
	{
		tCamera->Translate(-tCamera->GetRightVector() * Time::DeltaTime);
	}

	if (input->IsButtonDown(TF_KEY_Space))
	{
		tCamera->Translate(tCamera->GetUpVector() * Time::DeltaTime);
	}
	else if (input->IsButtonDown(TF_KEY_LeftShift))
	{
		tCamera->Translate(-tCamera->GetUpVector() * Time::DeltaTime);
	}

	return TF_OK;
}
