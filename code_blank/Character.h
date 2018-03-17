#pragma once

#include <PhysicsComponent.h>
#include <TransformComponent.h>
#include <AnimationComponent.h>
#include "CombatManager.h"
#include "GameplayAnimationMachine.h"

class Character
{
public:

	Character();
	~Character();

	void Init(bool, void*, CombatManagerDetails);

	// Virtual Functions
	//virtual void MoveReg(float, bool, tofu::math::float3, tofu::math::quat);
	//virtual void MoveAim(float, tofu::math::float3, tofu::math::quat, tofu::math::float3);
	//virtual void MoveEnemy(float, bool, tofu::math::float3);
	virtual void Update(float);
	virtual void UpdateState(float);

	virtual void Aim();
	virtual void Attack();
	virtual void Dodge();
	virtual void Die();

	void AnimationParameter(int _animationParameter);
	
	void CheckGroundStatus();
	void CurrentState(CharacterState _currentState);
	
	void HasEffect(bool _hasEffect);
	void HandleAirborneMovement(tofu::math::float3, tofu::math::float3, float);
	void HandleGroundedMovement(bool, tofu::math::float3, float);
	void LastState(CharacterState _lastState);
	virtual void Special(float, bool, bool);
	void Sprint(bool);
	void StateTimer(float _stateTimer);

	bool HasEffect();
	virtual bool IsDead();
	virtual bool IsGrounded();

	float StateTimer();

	CharacterState CurrentState();
	CharacterState LastState();

	// Shared Functions

	void ForceMove(float, float, int);
	void ForceMove(float, float, tofu::math::float3);

	void TakeDamage(float);

	void UseSpecial(float, bool, bool);

	CombatManager* GetCombatManager();
	std::string GetTag();
	tofu::math::float3 GetPosition();
	tofu::math::float3 GetForward();
	tofu::math::float3 GetRight();


	void SetComponents(tofu::TransformComponent, tofu::PhysicsComponent, tofu::AnimationComponent);

protected:

	std::string tag;

	tofu::TransformComponent	tCharacter;
	tofu::PhysicsComponent		pCharacter;
	tofu::AnimationComponent	aCharacter;
	tofu::PhysicsSystem*		physics;
	GameplayAnimationMachine*	gCharacter;

	CombatManager*				combatManager;

	CharacterState charState; // Do I need this one??
	CharacterState currentState;
	CharacterState lastState;

	// Movement
	float speed;
	bool isSprinting;

	// Player Stats
	float health;
	float moveSpeedMultiplier;
	float sprintSpeedMultiplier;
	float gravityMultiplier;
	float baseSpeedMultiplier;
	float slopeSpeedMultiplier = 0.18f;
	float airborneSpeedMultiplier;
	float jumpPower;	//[Range(1f, 20f)]
	float groundCheckDistance;

	int animationParameter;

	//void Move();

	// Bools
	bool isAiming;
	bool isGrounded;
	bool jump;
	bool sprinting;
	bool moving;
	bool hasJumped = false;
	bool isDead;
	bool hasEffect;
	bool queueJump;
	bool once;

	float turnMod;
	float origGroundCheckDistance;
	float stateTimer;
	float groundCheckRadius;
	float jumpTimer;
	float jumpDelay;

	tofu::math::float3 groundNormal;
	tofu::math::float3 move;

	tofu::math::quat charBodyRotation;
	tofu::math::quat rotation;

	// Constants
	const float kMaxSpeed = 12.0f;
	const float kAccelerate = 6.67f;
	const float kDeaccelerate = 10.0f;
	const float kAirDeaccelerate = 2.0f;
};