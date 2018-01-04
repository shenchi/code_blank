#include "Companion.h"
#include<cassert>
#include <RenderingComponent.h>

using namespace tofu;

Companion::Companion(tofu::math::float3 _target)
{
	{
		Entity e = Entity::Create();

		tComp = e.AddComponent<TransformComponent>();

		RenderingComponent r = e.AddComponent<RenderingComponent>();


		//anim = e.AddComponent<AnimationComponent>();

		Model* compModel = RenderingSystem::instance()->CreateModel("assets/cube.model");

		Material* cubeMat = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");


		cubeMat->SetTexture(diffuse);

		r->SetMaterial(cubeMat);
		r->SetModel(compModel);

		tComp->SetLocalPosition(math::float3{ 0.0f, 1.0f, 0.0f });
		tComp->SetLocalScale(math::float3{ 0.2f, 0.2f, 0.2f });
	}

	targetSet = false;
	inUse = false;

	SetTarget(_target);
	targetLastPos = _target;
}

Companion::~Companion() {}

// Update
// Delta Time, Target, TargetFoward
void Companion::Update(float dT, tofu::math::float3 _target, tofu::math::float3 _targetFwd)
{
	assert(targetSet);
	target = _target;
	targetFwd = _targetFwd;


	if (!inUse)
	{
		tofu::math::float3 dir = target - targetLastPos;
		tofu::math::float3 pos = tComp->GetLocalPosition();

		float wantedHeight = target.y + height;
		float currentHeight = pos.y;

		// TODO
		// Damp the height
		currentHeight = currentHeight + (heightDamping * (wantedHeight - currentHeight));	// Basic Lerp, make more robust later
		//currentHeight = Mathf.Lerp(currentHeight, wantedHeight, heightDamping * dT);
		
		
		
		// TODO
		// Set the position of the companion 
		tofu::math::float3 wantedPosition = target - targetFwd * distance;
		pos = pos + ((heightDamping * dT) * (wantedPosition - pos));	// Basic Lerp, make more robust later
		//transform.position = tofu::math::float3.Lerp(transform.position, wantedPosition, positionDamping * dT);


		// adjust the height of the companion
		pos.y = currentHeight;

		// look at the target

		//transform.forward = tofu::math::float3.Lerp(transform.forward, target.position - transform.position, rotationDamping * dT);

		tComp->SetLocalPosition(pos + dir);

		targetLastPos = target;
	}

	tofu::math::float3 wantedPosition;

	////check to see if there is anything behind the target
	//RaycastHit hit;
	//tofu::math::float3 back = transform.TransformDirection(-1 * tofu::math::float3.forward);

	//// cast the bumper ray out from rear and check to see if there is anything behind
	//if (Physics.Raycast(target.TransformPoint(bumperRayOffset), back, out hit, bumperDistanceCheck)
	//	&& hit.transform != target) // ignore ray-casts that hit the user. DR
	//{
	//	wantedPosition = transform.position;
	//	// clamp wanted position to hit position
	//	wantedPosition.x = hit.point.x;
	//	wantedPosition.z = hit.point.z;
	//	wantedPosition.y = Mathf.Lerp(hit.point.y + bumperHeight, wantedPosition.y, dt * damping);

	//	transform.position = tofu::math::float3.Lerp(transform.position, wantedPosition, dt * damping);
	//}
}

//-------------------------------------------------------------------------------------------------
// Setters

// 
void Companion::SetInUse(bool _inUse)
{
	inUse = _inUse;
}

// 
void Companion::SetTarget(tofu::math::float3 _target)
{
	target = _target;
	targetSet = true;
}