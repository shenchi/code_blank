#include "Camera.h"
#include <Entity.h>


using namespace tofu;

RayTestResult hitRay = {};

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
	cam->SetZNear(1.0f);
	cam->SetZFar(500.0f);

	distMod = math::float3{ 0.0f, 2.0f, 0.0f };

	distanceFromTarget = 5.0f;

	movedBack = false;

	SetSensitivity(2.0f);

	targetLastPos = math::float3{ 0.0f, 0.0f, 0.0f };

	physics = tofu::PhysicsSystem::instance();
	hitRay = {};
}

// Destructor
Camera::~Camera()
{}

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

// Fixed Update
void Camera::FixedUpdate(float fDT)
{ 
	camPos = camTarget + camRot * (math::float3{ 0.0f, 0.0f, -distanceFromTarget });
	SetPosition(camPos);
}

// Update
void Camera::Update(float dT)
{
	/*tCamera->SetLocalRotation(camRot);
	SetPosition(camPos);*/

	if (BumperCheck(camPos))
	{
		//float dist = (transform.position - target.transform.position).magnitude;
		float dist = math::length((camPos - camTarget));
		//float distance = ((hit.distance * 100) / bumperMaxDistance) * maxDistance;
		float distance = math::distance(camPos, hitRay.hitWorldPosition);
		//toTarget = Vector3.ClampMagnitude(toTarget, distance);
		if (!(dist < minDistance) & !movedBack)
		{
			float t = dT * damping;
			t = t*t*t * (t * (6.0f*t - 15.0f) + 10.0f);
			distanceFromTarget = math::mix(distanceFromTarget, 2.0f, t);
			movedBack = false;
		}
	}
	else
	{
		// Adjust if the camera is too close or too far away
		float dist = math::length((camPos - camTarget));
		if (dist > maxDistance)
		{
			float t = dT * damping;
			t = t*t*t * (t * (6.0f*t - 15.0f) + 10.0f);
			distanceFromTarget = math::mix(distanceFromTarget, 5.0f, t);
			movedBack = false;
		}
		else if (dist < minDistance + 1.9f)
		{
			math::float3 testVec = camTarget + camRot * (math::float3{ 0.0f, 0.0f, -(distanceFromTarget +1.2f) });;
			if (!BumperCheck(testVec))
			{
				float t = dT * damping;
				t = t*t*t * (t * (6.0f*t - 15.0f) + 10.0f);
				distanceFromTarget = math::mix(distanceFromTarget, 5.0f, t);
				//movedBack = true;
			}
			else
			{
				movedBack = false;
			}
		}
		else
		{
			movedBack = false;
		}
	}

	if (distanceFromTarget > 10) { distanceFromTarget = 10.0f; }
}

// Bumper check against surroundings
bool Camera::BumperCheck(tofu::math::float3 pos)
{
	RayTestResult hit = {};
	tofu::math::float3 back = -1.0f * tCamera->GetForwardVector();
	tofu::math::float3 right = tCamera->GetRightVector();
	tofu::math::float3 left = -1.0f * tCamera->GetRightVector();
	toTarget = (pos - camTarget);

	right = camPos + (right * bumperDistanceCheck);
	if(physics->RayTest(pos, right, &hit))
	{
		hitRay = hit;
		return true;
	}
	
	back = camPos + (back  * bumperDistanceCheck);
	if (physics->RayTest(pos, back, &hit))
	{
		hitRay = hit;
		return true;
	}

	left = camPos + (left  * bumperDistanceCheck);
	if (physics->RayTest(pos, left, &hit))
	{
		hitRay = hit;
		return true;
	}

	return false;
}

// Adjust the camera's position to account for hitting an object
//void Camera::AdjustCameraPosition(RayTestResult hit, tofu::math::float3 toTarget, float dT)
//{
//	//float dist = (transform.position - target.transform.position).magnitude;
//	float dist = math::length( (camPos - camTarget) );
//	//float distance = ((hit.distance * 100) / bumperMaxDistance) * maxDistance;
//	float distance = ((math::distance(camPos, hit.hitWorldPosition) * 100) / bumperMaxDistance) * maxDistance;
//	//toTarget = Vector3.ClampMagnitude(toTarget, distance);
//	toTarget = math::clamp(toTarget, 0.0f, distance);
//	if (!(dist < minDistance))
//	{
//		//transform.position = Vector3.Lerp(transform.position, (transform.position - toTarget), dT * damping);
//		// Custom Lerp funtion here
//		float t = adjustLerpTime / lerpTime;
//		t = t*t*t * (t * (6.0f*t - 15.0f) + 10.0f);
//		camPos = ((1 - t) * camPos) + (t * (camPos - toTarget));
//	}
//}

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
	assert(sen > 0.0f && sen < 4.0f);	// Change to value range for options screen
	sensitive = sen;
}

// Set Skybox
void Camera::SetSkybox(tofu::Material* skyBox)
{
	//cam->SetSkybox(skyB);
}

// Set Camera Target
void Camera::SetTarget(math::float3 target)
{
	camTarget = target + distMod;
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