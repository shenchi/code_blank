#pragma once

#include <PhysicsComponent.h>
#include <TransformComponent.h>
#include <AnimationComponent.h>

class Player
{
public:
	Player();
	~Player();

	void Act(float, bool, tofu::math::float3, tofu::math::quat);
	void Update();

	bool IsInAir();
	tofu::math::float3 GetPosition();

private:
	tofu::TransformComponent	tPlayer;
	tofu::PhysicsComponent		pPlayer;
	tofu::AnimationComponent	anim;

	// Movement
	float walkSpeed;
	float dashSpeed;
	float speed;
	bool inAir;
	bool isDashing;

	// Player Stats
	float health;

	const float kMaxSpeed = 2.0f;
	const float kAccelerate = 6.67f;
	const float kDeaccelerate = 10.0f;
};