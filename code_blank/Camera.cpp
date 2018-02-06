#include "Camera.h"
#include <Entity.h>


using namespace tofu;

// Constructor
Camera::Camera()
{
	// Create a camera
	{
		Entity e = Entity::Create();
		tCamera = e.AddComponent<TransformComponent>();
		cam = e.AddComponent<CameraComponent>();
	}

	SetPosition(math::float3{ 0, 0, -2 });
	SetFov(60.0f);

	distMod = math::float3{ 0.0f, 2.0f, 0.0f };

	SetSensitivity(0.01f);
}

// Destructor
Camera::~Camera()
{
}

// Move the Camera
void Camera::Move()
{
	camPos = camTarget + camRot * (math::float3{ 0.0f, 0.0f, -5.0f });
	SetPosition(camPos);
}

// Move the Camera, Float3 Override
void Camera::Move(math::float3 mov) {}

// Rotate the Camera
void Camera::Rotate(math::float2 rot)
{
	pitch += sensitive * rot.x;
	yaw += sensitive * rot.y;

	if (pitch < kMinPitch) pitch = kMinPitch;
	if (pitch > kMaxPitch) pitch = kMaxPitch;

	camRot = math::euler(pitch, yaw, 0.0f);
	tCamera->SetLocalRotation(camRot);
}

// Update
void Camera::Update()
{
	/*tCamera->SetLocalRotation(camRot);
	SetPosition(camPos);*/
}

void Camera::UpdateTarget(math::float3 target)
{
	camTarget = target + distMod;
	Move();
}

//-------------------------------------------------------------------------------------------------
// Setters

// Set Clear Color
void Camera::SetClearColor(math::float4 color)
{
	cam->SetClearColor(color);
}

// Set the distance from target modifier
void Camera::SetDistMod(math::float3 mod)
{
	distMod = mod;
}

// Set Field of View
void Camera::SetFov(float _fov)
{
	assert(_fov > 0.0f && _fov < 180.0f);	// Change to better values later
	fov = _fov;
	cam->SetFOV(fov);
}

// Set the Camera Position
void Camera::SetPosition(math::float3 pos)
{
	tCamera->SetLocalPosition(pos);
}

// Set Camera Rotation Sensitivity
void Camera::SetSensitivity(float sen)
{
	assert(sen > 0.0f && sen < 0.1f);	// Change to value range for options screen
	sensitive = sen;
}

// Set Skybox
void Camera::SetSkybox(tofu::Material* skyBox)
{
	cam->SetSkybox(skyBox);
}

//-------------------------------------------------------------------------------------------------
// Getters

// Get Camera Rotation
math::quat Camera::GetRotation()
{
	return camRot;
}

// Get Camera's Forward Vector
math::float3 Camera::GetForward()
{
	return tCamera->GetForwardVector();
}