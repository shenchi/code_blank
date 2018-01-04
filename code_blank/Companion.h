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

	void SetInUse(bool);
	void SetTarget(tofu::math::float3);

private:
	tofu::TransformComponent	tComp;
	tofu::AnimationComponent	anim;

	bool inUse;
	bool targetSet;

	tofu::math::float3 target;
	tofu::math::float3 targetLastPos;
	tofu::math::float3 targetFwd;


	float distance = 0.75f;
	float height = 1.7f;
	float heightDamping = 1.0f;
	float positionDamping = 2.0f;
	float rotationDamping = 1.0f;
};