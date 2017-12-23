#include <Engine.h>
#include "TestGame.h"

#include <RenderingSystem.h>
#include <InputSystem.h>

using namespace tofu;


namespace
{
	constexpr float MaxPitch = math::PI * 0.25f;
	constexpr float MinPitch = 0.0f;
	constexpr float InitPitch = math::PI * 0.125f;

	constexpr float Accelerate = 6.67f;
	constexpr float Deaccelerate = 10.0f;
	constexpr float WalkSpeed = 2.0f;
}

int32_t TestGame::Init()
{
	{
		Entity e = Entity::Create();

		tGround = e.AddComponent<TransformComponent>();

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		Model* model = RenderingSystem::instance()->CreateModel("assets/ground.model");

		Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::OpaqueMaterial);
		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/stone_wall_normalmap.texture");

		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);

		r->SetMaterial(material);
		r->SetModel(model);

		PhysicsComponent ph = e.AddComponent<PhysicsComponent>();
		ph->SetStatic(true);
		ph->SetBoxCollider(math::float3{ 25.0f, 0.5f, 25.0f });
		ph->SetColliderOrigin(math::float3{ 0.0f, -0.5f, 0.0f });
	}

	{
		Entity e = Entity::Create();

		tBox = e.AddComponent<TransformComponent>();
		tBox->SetLocalPosition(math::float3{ 0, 10, 10 });

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		Model* model = RenderingSystem::instance()->CreateModel("assets/cube.model");

		Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::OpaqueMaterial);
		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/stone_wall_normalmap.texture");

		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);

		r->SetMaterial(material);
		r->SetModel(model);

		PhysicsComponent ph = e.AddComponent<PhysicsComponent>();
	}

	{
		Entity e = Entity::Create();

		tPlayer = e.AddComponent<TransformComponent>();
		tPlayer->SetLocalPosition(math::float3{ 0.0f, 1.0f, 0.0f });
		tPlayer->SetLocalScale(math::float3{ 0.01f, 0.01f, 0.01f });

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		Model* model = RenderingSystem::instance()->CreateModel("assets/archer.model");

		anim = e.AddComponent<AnimationComponent>();

		Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::OpaqueSkinnedMaterial);
		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/archer_0.texture");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/archer_1.texture");

		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);

		r->SetMaterial(material);
		r->SetModel(model);

		pPlayer = e.AddComponent<PhysicsComponent>();

		pPlayer->LockRotation(true, false, true);
		pPlayer->SetCapsuleCollider(0.5f, 1.0f);
		pPlayer->SetColliderOrigin(math::float3{ 0.0f, 1.0f, 0.0f });
	}

	{
		Entity e = Entity::Create();

		tCamera = e.AddComponent<TransformComponent>();

		cam = e.AddComponent<CameraComponent>();
		
		cam->SetFOV(60.0f);
		tCamera->SetLocalPosition(math::float3{ 0, 0, -2 });

		Material* skyboxMat = RenderingSystem::instance()->CreateMaterial(MaterialType::SkyboxMaterial);
		TextureHandle tex = RenderingSystem::instance()->CreateTexture("assets/craterlake.texture");
		skyboxMat->SetTexture(tex);

		cam->SetSkybox(skyboxMat);
	}

	pitch = InitPitch;
	yaw = 0.0f;

	inAir = true;

	return TF_OK;
}

int32_t TestGame::Shutdown()
{
	return TF_OK;
}

int32_t TestGame::Update()
{
	inAir = !pPlayer->IsCollided();

	InputSystem* input = InputSystem::instance();
	if (input->IsButtonDown(ButtonId::TF_KEY_Escape))
	{
		Engine::instance()->Quit();
	}

	constexpr float sensitive = 0.01f;


	math::float3 inputDir = math::float3();

	if (input->IsGamepadConnected())
	{
		if (input->IsButtonDown(ButtonId::TF_GAMEPAD_FACE_RIGHT))
		{
			Engine::instance()->Quit();
		}

		inputDir.z = -input->GetLeftStickY();
		inputDir.x = input->GetLeftStickX();

		pitch += sensitive * input->GetRightStickY();
		yaw += sensitive * input->GetRightStickX();
	}

	pitch += sensitive * input->GetMouseDeltaY();
	yaw += sensitive * input->GetMouseDeltaX();

	if (pitch < MinPitch) pitch = MinPitch;
	if (pitch > MaxPitch) pitch = MaxPitch;


	if (input->IsButtonDown(TF_KEY_W))
	{
		inputDir.z = 1.0f;
	}
	else if (input->IsButtonDown(TF_KEY_S))
	{
		inputDir.z = -1.0f;
	}

	if (input->IsButtonDown(TF_KEY_D))
	{
		inputDir.x = 1.0f;
	}
	else if (input->IsButtonDown(TF_KEY_A))
	{
		inputDir.x = -1.0f;
	}

	bool jump = input->IsButtonDown(ButtonId::TF_KEY_Space) 
		|| input->IsButtonDown(ButtonId::TF_GAMEPAD_FACE_DOWN);

	math::quat camRot(pitch, yaw, 0.0f);
	math::float3 camTgt = tPlayer->GetLocalPosition() + math::float3{ 0.0f, 2.0f, 0.0f };
	math::float3 camPos = camTgt + camRot.rotate(math::float3{ 0.0f, 0.0f, -5.0f });
	
	tCamera->SetLocalPosition(camPos);
	tCamera->SetLocalRotation(camRot);

	float maxSpeed = WalkSpeed;

	if (math::length(inputDir) > 0.25f)
	{
		math::float3 moveDir = camRot.rotate(inputDir);
		moveDir.y = 0.0f;
		moveDir = math::normalize(moveDir);
		tPlayer->FaceTo(moveDir);

		speed += Time::DeltaTime * Accelerate;
		if (speed > maxSpeed)
			speed = maxSpeed;

		tPlayer->Translate(moveDir * Time::DeltaTime * speed);

		anim->CrossFade(1, 0.3f);
	}
	else
	{
		speed -= Time::DeltaTime * Deaccelerate;
		if (speed < 0.0f) speed = 0.0f;
		tPlayer->Translate(tPlayer->GetForwardVector() * Time::DeltaTime * speed);

		anim->CrossFade(0, 0.2f);
	}

	if (jump && !inAir)
	{
		pPlayer->ApplyImpulse(math::float3{ 0.0f, 2.0f, 0.0f });
	}

	return TF_OK;
}
