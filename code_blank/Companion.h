#pragma once

#include <AnimationComponent.h>
#include <TransformComponent.h>
#include<TofuMath.h>

// Player Companion Class
// Sword/Gun for the player to use in combat
// Also may act as the map displayer, among other possible features
class Companion
{
public:
	Companion(tofu::math::float3);
	~Companion();

	void Update(float, tofu::math::float3, tofu::math::float3);
	void FixedUpdate(float, tofu::math::float3, tofu::math::float3);

	void SetInUse(bool);
	void SetActive(bool);
	void SetTarget(tofu::math::float3);

	bool ActiveSelf();
	tofu::math::float3 GetPosition();

private:
	tofu::TransformComponent	tComp;
	tofu::AnimationComponent	aComp;

	bool inUse;
	bool targetSet;
	bool isActive;

	tofu::math::float3 target;
	tofu::math::float3 targetLastPos;
	tofu::math::float3 targetFwd;


	float distance = 50.0f;
	float height = 1.7f;
	float heightDamping = 1.0f;
	float positionDamping = 2.0f;
	float rotationDamping = 1.0f;
};