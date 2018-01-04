#pragma once

#include <PhysicsComponent.h>
#include <TransformComponent.h>
#include <AnimationComponent.h>

class Player
{
public:
	Player();
	~Player();

	void MoveReg(float, bool, tofu::math::float3, tofu::math::quat);
	void MoveAim(float, tofu::math::float3, tofu::math::quat, tofu::math::float3);
	void Update();

	void Aim();
	void Attack();
	void Dash();
	void Dodge();
	void Interact();
	void Special();
	void VisionHack();

	bool IsInAir();
	tofu::math::float3 GetPosition();
	tofu::math::float3 GetForward();

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

	const float kMaxSpeed = 5.0f;
	const float kAccelerate = 6.67f;
	const float kDeaccelerate = 10.0f;

	//void Move();
};