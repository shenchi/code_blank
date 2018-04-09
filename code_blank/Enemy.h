#pragma once

#include "Character.h"

class Enemy : public Character
{
public:
	Enemy(CharacterDetails, void*, void*);
	~Enemy();

	void MoveEnemy(float, bool, tofu::math::float3);
	//void MoveReg(float, bool, tofu::math::float3, tofu::math::quat);
	//void MoveAim(float, tofu::math::float3, tofu::math::quat, tofu::math::float3);
	void Update(float);
	void UpdateState(float);

	void Aim();
	//void AnimationParameter(int _animationParameter);
	void Attack();
	//void CheckGroundStatus();
	//void CurrentState(CharacterState _currentState);
	//void Dodge();
	void Die();
	//void ForceMove(float, float, int);
	//void ForceMove(float, float, tofu::math::float3);
	//void Special();



private:
	tofu::TransformComponent	tEnemy;
	tofu::PhysicsComponent		pEnemy;
	tofu::AnimationComponent	aEnemy;
	GameplayAnimationMachine*	gEnemy;
	tofu::PhysicsSystem*		physics;

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

	// Audio
	/*
	AudioSource charAudio;
	AudioSource charCombatAudio;
	AudioClip footsteps1;
	AudioClip footsteps2;
	AudioClip footsteps3;
	AudioClip footsteps4;
	AudioClip jumpFX;
	AudioClip landFX;
	*/
};