#include "CharacterControllerJam.h"

#include "PhysicsSystem.h"

using namespace tofu;

typedef const rapidjson::Value& value_t;

namespace
{
	constexpr float MaxPitch = math::PI * 0.25f;
	constexpr float MinPitch = 0.0f;
	constexpr float InitPitch = math::PI * 0.125f;

	constexpr float Accelerate = 6.67f;
	constexpr float Deaccelerate = 10.0f;
	constexpr float RunSpeed = 4.0f;

	constexpr float JumpingUpInitialSpeed = 10.0f;

}

int32_t CharacterControllerJam::Init()
{
	CHECKED(sceneMgr.Init());

	CHECKED(sceneMgr.LoadScene("assets/scenes/CharacterControllerJam.json"));

	{
		Entity e = Entity::Create();

		tPlayer = e.AddComponent<TransformComponent>();
		tPlayer->SetLocalPosition(math::float3{ 0.0f, 8.0f, 0.0f });
		tPlayer->SetLocalScale(math::float3{ 0.01f, 0.01f, 0.01f });

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		Model* model = RenderingSystem::instance()->CreateModel("assets/akai/akai.model");

		anim = e.AddComponent<AnimationComponent>();

		AddAnimState("Idle");
		AddAnimState("Running");
		AddAnimState("RunningTurn", false);
		AddAnimState("JumpingUp", false);
		AddAnimState("JumpingUp2", false);
		AddAnimState("Falling");
		AddAnimState("Landing", false);
		AddAnimState("Landing2", false);
		AddAnimState("Landing3", false);
		AddAnimState("LandToEdge", false);
		AddAnimState("LandToEdge2", false);
		AddAnimState("Hanging");
		AddAnimState("HangingClimbUp", false);
		AddAnimState("StandingUp", false);
		AddAnimState("HangingDrop", false);
		AddAnimState("HangingJump", false);
		AddAnimState("HangingLeft");
		AddAnimState("HangingRight");
		AddAnimState("DropToHanging", false);
		AddAnimState("HangingToBraced", false);

		Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaqueSkinned);

		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/archer_0.texture");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/archer_1.texture");

		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);

		r->SetMaterial(material);
		r->SetModel(model);

		pPlayer = e.AddComponent<PhysicsComponent>();

		pPlayer->LockRotation(true, true, true);
		pPlayer->SetCapsuleCollider(50.0f, 80.0f);
		pPlayer->SetColliderOrigin(math::float3{ 0.0f, 120.0f, 0.0f });

		pPlayer->SetGravity(math::float3{});
	}

	// camera
	{
		Entity e = Entity::Create();

		tCamera = e.AddComponent<TransformComponent>();

		cam = e.AddComponent<CameraComponent>();

		cam->SetFOV(60.0f);
		tCamera->SetLocalPosition(math::float3{ 0, 0, -2 });

		TextureHandle tex = RenderingSystem::instance()->CreateTexture("assets/textures/test/darkcity - Copy.texture");
		TextureHandle skyboxDiff = RenderingSystem::instance()->CreateTexture("assets/textures/test/diffuseIrradianceMapd - Copy.texture");
		TextureHandle skyboxSpec = RenderingSystem::instance()->CreateTexture("assets/textures/test/prefilteredMapd - Copy.texture");

		cam->SetSkybox(tex);
		cam->SetSkyboxDiffuseMap(skyboxDiff);
		cam->SetSkyboxSpecularMap(skyboxSpec);
	}

	pitch = InitPitch;
	yaw = 0.0f;

	return kOK;
}

int32_t CharacterControllerJam::Shutdown()
{
	return kOK;
}

int32_t CharacterControllerJam::Update()
{
	return kOK;
}

int32_t CharacterControllerJam::FixedUpdate()
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
		if (input->IsButtonDown(ButtonId::kGamepadFaceRight))
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

	if (input->IsButtonDown(kKeyD))
	{
		inputDir.x = 1.0f;
	}
	else if (input->IsButtonDown(kKeyA))
	{
		inputDir.x = -1.0f;
	}

	bool jump = input->IsButtonDown(ButtonId::kKeySpace)
		|| input->IsButtonDown(ButtonId::kGamepadFaceDown);

	math::float3 playerPos = pPlayer->GetPosition();
	math::quat playerRot = pPlayer->GetRotation();

	math::quat camRot = math::euler(pitch, yaw, 0.0f);
	math::float3 camTgt = playerPos + math::float3{ 0.0f, 2.0f, 0.0f };
	math::float3 camPos = camTgt + camRot * (math::float3{ 0.0f, 0.0f, -5.0f });

	tCamera->SetLocalPosition(camPos);
	tCamera->SetLocalRotation(camRot);

	PhysicsSystem& phys = *PhysicsSystem::instance();

	{
		math::float3 rayStart = playerPos + math::float3{ 0, 1, 0 };
		math::float3 rayEnd = playerPos + math::float3{ 0, -0.04f, 0 };

		inAir = !phys.RayTest(rayStart, rayEnd);
	}

	float maxSpeed = RunSpeed;

	math::float3 lastVelocity = pPlayer->GetVelocity();
	math::float3 velocity{};

	bool hasInputDir = math::length(inputDir) > 0.25f;

	math::float3 fwd{ 0, 0, 1 };
	if (hasInputDir && !inAir)
	{
		math::float3 moveDir = camRot * inputDir;
		moveDir.y = 0.0f;
		moveDir = math::normalize(moveDir);

		{
			math::float3 faceDir = -moveDir;
			float cosTheta = math::dot(faceDir,  fwd);
			if (cosTheta >= 1.0 - FLT_EPSILON)
			{
				playerRot = math::quat();
			}
			else if (-cosTheta >= 1.0 - FLT_EPSILON)
			{
				playerRot = { 0, 0, 1, 0 };
			}
			else
			{
				float angle = math::acos(cosTheta);
				math::float3 axis = math::normalize(math::cross(fwd, faceDir));
				playerRot = math::angleAxis(angle, axis);
			}
			pPlayer->SetRotation(playerRot);
		}

		speed += Time::FixedDeltaTime * Accelerate;
		if (speed > maxSpeed)
			speed = maxSpeed;

		velocity = moveDir * speed;
	}
	else if (!inAir)
	{
		speed -= Time::FixedDeltaTime * Deaccelerate;
		if (speed < 0.0f) speed = 0.0f;
		velocity = -(playerRot * fwd) * speed;
	}

	// falling
	if (inAir)
	{
		velocity.y = lastVelocity.y - 10.0f * Time::FixedDeltaTime;
	}
	else
	{
		velocity.y = 0;
	}
	
	switch (state)
	{
	case kStateIdle:
		if (inAir)
		{
			state = kStateFalling;
			anim->CrossFade("Falling", 0.1f);
		}
		else if (hasInputDir)
		{
			state = kStateRunning;
			anim->CrossFade("Running", 0.1f);
		}
		else if (jump)
		{
			state = kStateJumpingPrepare;
			anim->CrossFade("JumpingUp", 0.1f);
		}
		break;
	case kStateFalling:
		if (!inAir)
		{
			state = kStateIdle;
			anim->CrossFade("Idle", 0.1f);
		}
		break;
	case kStateRunning:
		if (inAir)
		{
			state = kStateFalling;
			anim->CrossFade("Falling", 0.1f);
		}
		else if (!hasInputDir)
		{
			state = kStateIdle;
			anim->CrossFade("Idle", 0.2f);
		}
		break;
	case kStateJumpingPrepare:
		if (anim->GetProgress() > 0.5f)
		{
			state = kStateJumpingUp;
			anim->CrossFade("Falling", 0.5f);
			velocity.y = JumpingUpInitialSpeed;
			inAir = true;
		}
		break;
	case kStateJumpingUp:
		if (!inAir)
		{
			velocity.y = JumpingUpInitialSpeed;
			inAir = true;
		}
		else 
		{
			state = kStateFalling;
		}
		break;
	}

	pPlayer->SetVelocity(velocity);

	return kOK;
}

void CharacterControllerJam::AddAnimState(const char * name, bool isLoop, const char * clipname)
{
	if (nullptr == clipname) clipname = name;

	AnimationStateMachine *stateMachine = anim->GetStateMachine();
	AnimationState *newState = stateMachine->AddState(name, isLoop);
	newState->animationName = clipname;
}
