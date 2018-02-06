#pragma once

#include <PhysicsComponent.h>
#include <TransformComponent.h>
#include <AnimationComponent.h>
#include "GameplayAnimationMachine.h"

class Player
{
public:
	Player(void*);
	~Player();

	void MoveReg(float, bool, tofu::math::float3, tofu::math::quat);
	void MoveAim(float, tofu::math::float3, tofu::math::quat, tofu::math::float3);
	void Update(float);

	void Aim();
	void Attack();
	void Dodge();
	void Interact();
	void Special();
	void Sprint(bool);
	void VisionHack();

	bool IsInAir();
	tofu::math::float3 GetPosition();
	tofu::math::float3 GetForward();

private:
	tofu::TransformComponent	tPlayer;
	tofu::PhysicsComponent		pPlayer;
	tofu::AnimationComponent	aPlayer;
	GameplayAnimationMachine*	gPlayer;
	tofu::PhysicsSystem*		physics;
	CombatManager*				combatManager;

	// Movement
	float walkSpeed;
	float sprintSpeed;
	float speed;
	bool inAir;
	bool isSprinting;

	// Player Stats
	float health;

	const float kMaxSpeed = 12.0f;
	const float kAccelerate = 6.67f;
	const float kDeaccelerate = 10.0f;

	//void Move();
};