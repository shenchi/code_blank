#include "AnimationDemo.h"

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

int32_t AnimationDemo::Init()
{
	{
		Entity eBox = Entity::Create();
		Entity &e = eBox;

		tBox = e.AddComponent<TransformComponent>();
		tBox->SetLocalPosition(math::float3{ 53.0f, 10.0f, -38.0f });
		tBox->SetLocalScale(math::float3{ 0.1f, 0.1f, 0.1f });

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		Model* model = RenderingSystem::instance()->CreateModel("assets/cube.model");

		Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/stone_wall_normalmap.texture");

		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);

		r->SetMaterial(material);
		r->SetModel(model);
	}

	{
		Entity e = Entity::Create();

		tPlayer = e.AddComponent<TransformComponent>();
		//tPlayer->SetLocalPosition(math::float3{ -5.0f, 8.0f, -5.0f });
		tPlayer->SetLocalPosition(math::float3{ 53.0f, 8.0f, -38.0f });
		//tPlayer->SetLocalPosition(math::float3{ 0.0f, 8.0f, 0.0f });
		tPlayer->SetLocalScale(math::float3{ 0.01f, 0.01f, 0.01f });

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		Model* model = RenderingSystem::instance()->CreateModel("assets/archer.model");
		//Model* model = RenderingSystem::instance()->CreateModel("assets/archer_test.model");
		//Model* model = RenderingSystem::instance()->CreateModel("assets/soldier.model");

		anim = e.AddComponent<AnimationComponent>();

		AnimationStateMachine *stateMachine = anim->GetStateMachine();

		AnimationState *idle = stateMachine->AddState("idle");
		idle->animationName = "idle";
		AnimationState *walk = stateMachine->AddState("walk");
		walk->animationName = "walk";

		// IK test
		anim->SetIKTarget(IKTarget::LeftHand, 2, &eBox);
		//anim->SetIKTarget(IKTarget::LeftFoot, 2, &eBox);

		Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaqueSkinned);

		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/archer_0.texture");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/archer_1.texture");
		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);

		r->SetMaterial(material);
		r->SetModel(model);
	}

	// Dummy light
	{
		Entity e = Entity::Create();

		tSun = e.AddComponent<TransformComponent>();
		tSun->SetLocalPosition(math::float3{ 58.0f, 3.0f, -41.0f });
		tSun->SetLocalRotation(math::angleAxis(3.14f / 4, math::float3{ 1.0f, 0.0f, 0.0f }));

		lSun = e.AddComponent<LightComponent>();
		lSun->SetType(LightType::kLightTypeDirectional);
		math::float4 sunColor = math::float4{ 1.0f, 1.0f, 1.0f, 1.0f };
		lSun->SetColor(sunColor);
		lSun->SetCastShadow(true);
	}
	// Moon light 
	{
		Entity e = Entity::Create();

		tMoon = e.AddComponent<TransformComponent>();

		tMoon->SetLocalRotation(math::angleAxis(3.14f / 2.0f, math::float3{ 1.0f,0.0f, 0.0f }));

		lMoon = e.AddComponent<LightComponent>();
		lMoon->SetType(LightType::kLightTypeDirectional);
		math::float4 moonColor = math::float4{ 1.0f, 1.0f, 1.0f, 1.0f };
		lMoon->SetColor(moonColor);
	}
	// Point light
	{
		Entity e = Entity::Create();

		tBulb = e.AddComponent<TransformComponent>();
		tBulb->SetLocalPosition(math::float3{ -5, 5, -4 });

		lBulb = e.AddComponent<LightComponent>();
		lBulb->SetType(LightType::kLightTypePoint);
		math::float4 bulbColor = math::float4{ 1.0f, 1.0f, 1.0f, 1.0f };
		lBulb->SetColor(bulbColor);
	}

	// camera
	{
		Entity e = Entity::Create();

		tCamera = e.AddComponent<TransformComponent>();

		cam = e.AddComponent<CameraComponent>();

		cam->SetFOV(60.0f);
		tCamera->SetLocalPosition(math::float3{ 0, 0, -2 });
		//	tCamera->SetLocalPosition(math::float3{ 0, 100, -2 });

		Material* skyboxMat = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeSkybox);
		TextureHandle tex = RenderingSystem::instance()->CreateTexture("assets/craterlake.texture");
		skyboxMat->SetTexture(tex);

		cam->SetSkybox(skyboxMat);
	}

	pitch = InitPitch;
	yaw = 0.0f;

	return kOK;
}

int32_t AnimationDemo::Shutdown()
{
	return kOK;
}

int32_t AnimationDemo::Update()
{
	InputSystem* input = InputSystem::instance();
	if (input->IsButtonDown(ButtonId::kKeyEscape))
	{
		Engine::instance()->Quit();
	}

	constexpr float sensitive = 0.01f;


	math::float3 inputDir = math::float3();

	if (input->IsGamepadConnected())
	{
		if (input->IsButtonDown(ButtonId::kGamepadStart))
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


	if (input->IsButtonDown(kKeyW))
	{
		inputDir.z = 1.0f;
	}
	else if (input->IsButtonDown(kKeyS))
	{
		inputDir.z = -1.0f;
	}

	if (input->IsButtonDown(kKeyI))
	{
		inputDir.y = 1.0f;
	}
	else if (input->IsButtonDown(kKeyK))
	{
		inputDir.y = -1.0f;
	}

	if (input->IsButtonDown(kKeyD))
	{
		inputDir.x = 1.0f;
	}
	else if (input->IsButtonDown(kKeyA))
	{
		inputDir.x = -1.0f;
	}

	math::quat camRot = math::euler(pitch, yaw, 0.0f);
	math::float3 camTgt = tPlayer->GetLocalPosition() + math::float3{ 0.0f, 2.0f, 0.0f };
	math::float3 camPos = camTgt + camRot * (math::float3{ 0.0f, 0.0f, -5.0f });

	tCamera->SetLocalPosition(camPos);
	tCamera->SetLocalRotation(camRot);

	float maxSpeed = WalkSpeed;

	anim->CrossFade("idle", 0.1f);

	if (math::length(inputDir) > 0.25f)
	{
		math::float3 moveDir = camRot * inputDir;
		moveDir = math::normalize(moveDir);
		
		speed += Time::DeltaTime * Accelerate;
		if (speed > maxSpeed)
			speed = maxSpeed;

		tBox->Translate(moveDir * Time::DeltaTime * speed);
	}
	else
	{
		speed -= Time::DeltaTime * Deaccelerate;
		if (speed < 0.0f) speed = 0.0f;
		tBox->Translate(tPlayer->GetForwardVector() * Time::DeltaTime * speed);
	}

	return kOK;
}
