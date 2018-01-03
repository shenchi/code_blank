#pragma once


#include <TransformComponent.h>
#include <CameraComponent.h>

// Camera class
// Has a camera component and takes input from PController
class Camera
{
public:
	Camera();
	~Camera();

	// TODO
	// move, rotate, 
	void Move();
	void Move(tofu::math::float3);
	void Rotate(tofu::math::float2);
	void Update();
	void UpdateTarget(tofu::math::float3);

	tofu::math::quat GetRotation();

	void SetClearColor(tofu::math::float4);
	void SetDistMod(tofu::math::float3);
	void SetFov(float);
	void SetPosition(tofu::math::float3);
	void SetSensitivity(float);
	void SetSkybox(tofu::Material*);

private:
	tofu::CameraComponent cam;
	tofu::TransformComponent tCamera;

	tofu::math::float3 camTarget;
	tofu::math::quat camRot;
	tofu::math::float3 camPos;
	tofu::math::float3 distMod;

	float fov;
	float pitch;
	float yaw;
	float sensitive;

	const float kMaxPitch = tofu::math::PI * 0.25f;
	const float kMinPitch = tofu::math::PI * -0.125f;
};