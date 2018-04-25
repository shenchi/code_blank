#pragma once

#include <PhysicsComponent.h>
#include <TransformComponent.h>
#include <AnimationComponent.h>
#include "CombatManager.h"
#include "GameplayAnimationMachine.h"
#include <AudioManager.h>

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

	virtual void Attack();
	virtual void Dodge();
	virtual void Die();

	void AnimationParameter(int _animationParameter);
	
	void CheckGroundStatus();
	void CurrentState(CharacterState _currentState);
	
	void HasEffect(bool _hasEffect);
	void HandleAirborneMovement(tofu::math::float3, bool, float);
	void HandleGroundedMovement(tofu::math::float3, bool, bool, float);
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
	void ForceMove(float, float);
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
	float GetAnimationDuration(CharacterState state);

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

	tofu::AudioSource			footstep1_SFX{ "assets/sounds/Footstep01.wav" };
	tofu::AudioSource			footstep2_SFX{ "assets/sounds/Footstep02.wav" };
	tofu::AudioSource			jump_SFX{ "assets/sounds/Jump.wav" };
	tofu::AudioSource			land_SFX{ "assets/sounds/Land.wav" };

	// Player Stats
	float health;
	float maxHealth;
	float energy;
	float maxEnergy;
	float stamina;
	float maxStamina;
	float staminaRegen;
	float energyRegen;
	float groundCheckDistance;

	int animationParameter;

	// Movement
	float speed;
	float baseSpeedMultiplier;
	
	float moveSpeedMultiplier;
	float sprintSpeedMultiplier;
	float gravityMultiplier = 7.0f;
	float airbornMaxVelocity = 2.0f;
	float slopeSpeedMultiplier = 0.18f;
	float airborneSpeedMultiplier;
	float jumpPower;	//[Range(1f, 20f)]

	// Bools
	bool isAiming;
	bool isGrounded;
	bool isRolling;
	bool isSprinting;
	bool inAir;
	bool jump;
	bool moving;
	bool hasJumped = false;
	bool isDead;
	bool hasEffect;
	bool once;

	float turnMod;
	float origGroundCheckDistance;
	float stateTimer;
	float groundCheckRadius;
	float lerpMod = 0.01f;
	float vertLerp = 0.35f;
	float horiLerp = 0.35f;
	float deathTimer = 3.0f;

	tofu::math::float3 groundNormal;
	tofu::math::float3 move;
	tofu::math::float3 lastVelocity;
	tofu::math::float3 velocity;
	tofu::math::quat charRot;

	tofu::math::quat charBodyRotation;
	tofu::math::quat rotation;

	// Constants
	const float kMaxSpeed = 8.0f;			// 12.0f
	const float kAccelerate = 4.0f;		// 6.67f
	const float kDeaccelerate = 10.0f;		// 10.0f
	const float kAirDeaccelerate = 2.0f;	// 2.0f
};