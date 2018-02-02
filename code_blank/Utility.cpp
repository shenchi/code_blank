#include "Utility.h"

using namespace tofu;

namespace
{
	constexpr float MaxPitch = math::PI * 0.5f;
	constexpr float MinPitch = -math::PI * 0.5f;

	constexpr float Accelerate = 6.67f;
	constexpr float Deaccelerate = 10.0f;
	constexpr float WalkSpeed = 3.0f; 
	constexpr float RunSpeed = 6.0f;
}

namespace Utility
{

	GhostPlayer::GhostPlayer(tofu::math::float3 pos, tofu::Material* skybox)
	{
		entity = Entity::Create();

		transform = entity.AddComponent<TransformComponent>();
		transform->SetLocalPosition(pos);

		camera = entity.AddComponent<CameraComponent>();
		camera->SetFOV(60.0f);
		camera->SetSkybox(skybox);

		pitch = 0.0f;
		yaw = 0.0f;
	}

	GhostPlayer::~GhostPlayer()
	{
	}

	int32_t GhostPlayer::Update()
	{
		InputSystem* input = InputSystem::instance();


		constexpr float sensitive = 0.01f;


		math::float3 inputDir = math::float3();

		if (input->IsGamepadConnected())
		{
			if (input->IsButtonDown(ButtonId::kGamepadFaceRight))
			{
				Engine::instance()->Quit();
			}

			inputDir.z = -input->GetLeftStickY();
			inputDir.y = input->GetLeftTrigger() - input->GetRightTrigger();
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

		if (input->IsButtonDown(kKeyE) || input->IsButtonDown(kKeySpace))
		{
			inputDir.y = 1.0f;
		}
		else if (input->IsButtonDown(kKeyQ))
		{
			inputDir.y = -1.0f;
		}

		math::quat camRot = math::euler(pitch, yaw, 0.0f);
		transform->SetLocalRotation(camRot);
		//math::float3 camTgt = tPlayer->GetLocalPosition() + math::float3{ 0.0f, 2.0f, 0.0f };
		//math::float3 camPos = camTgt + camRot * (math::float3{ 0.0f, 0.0f, -5.0f });

		float maxSpeed = WalkSpeed;
		if (input->IsButtonDown(kKeyLeftShift))
		{
			maxSpeed = RunSpeed;
		}

		if (math::length(inputDir) > 0.25f)
		{
			math::float3 moveDir = 
				transform->GetForwardVector() * inputDir.z +
				transform->GetRightVector() * inputDir.x +
				transform->GetUpVector() * inputDir.y;

			//moveDir.y = 0.0f;
			moveDir = math::normalize(moveDir);
			//tPlayer->FaceTo(-moveDir);

			speed += Time::DeltaTime * Accelerate;
			if (speed > maxSpeed)
				speed = maxSpeed;

			transform->Translate(moveDir * Time::DeltaTime * speed);

		}
		else
		{
			speed -= Time::DeltaTime * Deaccelerate;
			if (speed < 0.0f) speed = 0.0f;
			transform->Translate(transform->GetForwardVector() * Time::DeltaTime * speed);
		}

		return kOK;
	}

}