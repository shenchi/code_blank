#pragma once

#include <PhysicsComponent.h>
#include <TransformComponent.h>
#include <AnimationComponent.h>

class Enemy
{
public:
	Enemy(tofu::math::float3);
	~Enemy();

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
	tofu::TransformComponent	tEnemy;
	tofu::PhysicsComponent		pEnemy;
	tofu::AnimationComponent	aEnemy;

	// Movement
	float walkSpeed;
	float dashSpeed;
	float speed;
	bool inAir;
	bool isDashing;

	// Enemy Stats
	float health;

	const float kMaxSpeed = 5.0f;
	const float kAccelerate = 6.67f;
	const float kDeaccelerate = 10.0f;

	//void Move();
};