#pragma once

#include <PhysicsComponent.h>
#include <TransformComponent.h>
#include <AnimationComponent.h>
#include "GameplayAnimationMachine.h"

class Character
{
public:

	Character();
	~Character();

	virtual void MoveReg(float, bool, tofu::math::float3, tofu::math::quat);
	virtual void MoveAim(float, tofu::math::float3, tofu::math::quat, tofu::math::float3);
	virtual void Update(float);
	virtual void UpdateState();

	virtual void Aim();
	virtual void AnimationParameter(int _animationParameter);
	virtual void Attack();
	virtual void CheckGroundStatus();
	virtual void CurrentState(CharacterState _currentState);
	virtual void Dodge();
	virtual void ForceMove(float, float, int);
	virtual void ForceMove(float, float, tofu::math::float3);
	virtual void HasEffect(bool _hasEffect);
	virtual void HandleAirborneMovement(float, float);
	virtual void HandleGroundedMovement(bool);
	virtual void LastState(CharacterState _lastState);
	virtual void Special();
	virtual void Sprint(bool);
	virtual void StateTimer(float _stateTimer);

	virtual bool HasEffect();
	virtual bool IsDead();
	virtual bool IsGrounded();
	virtual bool IsInAir();

	virtual float StateTimer();

	virtual tofu::math::float3 GetPosition();
	virtual tofu::math::float3 GetForward();

	virtual CharacterState CurrentState();
	virtual CharacterState LastState();

protected:

	CombatManager*				combatManager;

	CharacterState charState; // Do I need this one??
	CharacterState currentState;
	CharacterState lastState;

	// Movement
	float speed;
	bool inAir;
	bool isSprinting;

	// Player Stats
	float health;
	float moveSpeedMultiplier;
	float sprintSpeedMultiplier;
	float gravityMultiplier;
	float baseSpeedMultiplier;
	float slopeSpeedMultiplier = 0.18f;
	float jumpPower;	//[Range(1f, 20f)]
	float groundCheckDistance;

	int animationParameter;

	//void Move();

	// Bools
	bool isGrounded;
	bool jump;
	bool sprinting;
	bool moving;
	bool hasJumped = false;
	bool isDead;
	bool hasEffect;


	float turnMod;
	float origGroundCheckDistance;
	float stateTimer;
	float groundCheckRadius;

	tofu::math::float3 groundNormal;
	tofu::math::float3 move;

	tofu::math::quat charBodyRotation;
	tofu::math::quat rotation;

	// Constants
	const float kMaxSpeed = 12.0f;
	const float kAccelerate = 6.67f;
	const float kDeaccelerate = 10.0f;
};