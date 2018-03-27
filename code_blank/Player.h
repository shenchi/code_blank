#pragma once

#include "Character.h"
#include "Gun.h"

class Player : public Character
{
public:
	Player(CharacterDetails, void*);
	~Player();

	void MoveReg(float, bool, tofu::math::float3, tofu::math::quat);
	void MoveAim(float, tofu::math::float3, tofu::math::quat, tofu::math::float3);
	//void MoveEnemy(float, bool, tofu::math::float3);
	void Update(float);
	void FixedUpdate(float);
	void UpdateState(float);


	void Aim(bool);
	void Attack(bool, float);
	void Dodge(tofu::math::float3);
	void Die();
	void Interact();
	void Special(bool, float);
	void VisionHack();

	bool AimList();

	/*void Aim();
	void AnimationParameter(int _animationParameter);
	
	void CheckGroundStatus();
	void CurrentState(CharacterState _currentState);
	
	
	void ForceMove(float, float, int);
	void ForceMove(float, float, tofu::math::float3);
	void HasEffect(bool _hasEffect);
	void HandleAirborneMovement(tofu::math::float3);
	void HandleGroundedMovement(bool);
	
	void LastState(CharacterState _lastState);
	
	void Sprint(bool);
	void StateTimer(float _stateTimer);
	

	bool HasEffect();
	bool IsDead();
	bool IsGrounded();

	float StateTimer();*/

	/*tofu::math::float3 GetPosition();
	tofu::math::float3 GetForward();
	tofu::math::float3 GetRight();*/

	//CharacterState CurrentState();
	//CharacterState LastState();

private:
	tofu::TransformComponent	tPlayer;
	tofu::PhysicsComponent		pPlayer;
	tofu::AnimationComponent	aPlayer;
	tofu::PhysicsSystem*		physics;
	
	tofu::math::quat camRotation;

	tofu::math::float3 move;
	tofu::math::float3 lastMove;

	Gun* gun;

	bool attackButtonDown;
	bool specialButtonDown;
	float attackButtonTimer;
	float minHoldTime;
	float maxHoldTime;
	float specialButtonTimer;
	float rollDodgeCost;


	//GameObject charBody;
	//GameObject camera;

	
	//CapstoneAnimation animator;

	//Rigidbody rigidbody;


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
