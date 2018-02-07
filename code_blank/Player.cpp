#include "Player.h"
#include <PhysicsSystem.h>
#include <RenderingComponent.h>

using namespace tofu;

Player::Player(CharacterDetails details, void* comp)
{
	{
		Entity e = Entity::Create();

		tPlayer = e.AddComponent<TransformComponent>();
		tPlayer->SetLocalPosition(math::float3{ 53.0f, 8.0f, -38.0f });
		tPlayer->SetLocalScale(math::float3{ 0.01f, 0.01f, 0.01f });

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		Model* model = RenderingSystem::instance()->CreateModel("assets/archer.model");

		aPlayer = e.AddComponent<AnimationComponent>();

		AnimationStateMachine *stateMachine = aPlayer->GetStateMachine();

		AnimationState *idle = stateMachine->AddState("idle");
		idle->animationName = "idle";
		AnimationState *walk = stateMachine->AddState("walk");
		walk->animationName = "walk";

		AnimationState *jump = stateMachine->AddState("jump");
		jump->animationName = "jump";
		AnimationState *run = stateMachine->AddState("run");
		run->animationName = "run";
		

		Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaqueSkinned);
		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/archer_0.texture");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/archer_1.texture");

		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);

		r->SetMaterial(material);
		r->SetModel(model);

		pPlayer = e.AddComponent<PhysicsComponent>();

		pPlayer->LockRotation(true, false, true);
		pPlayer->SetCapsuleCollider(50.0f, 100.0f);
		pPlayer->SetColliderOrigin(math::float3{ 0.0f, 100.0f, 0.0f });
	}

	moveSpeedMultiplier = 5.0f;
	sprintSpeedMultiplier = 10.0f;

	jumpPower = 4.0f;

	stateTimer = 0;
	
	physics = tofu::PhysicsSystem::instance();

	combatManager = new CombatManager(true, comp, this);

	gPlayer = new GameplayAnimationMachine(aPlayer, combatManager);


	//rigidbody = GetComponent<Rigidbody>();
	//rigidbody.constraints = RigidbodyConstraints.FreezeRotationX | RigidbodyConstraints.FreezeRotationY | RigidbodyConstraints.FreezeRotationZ;
	origGroundCheckDistance = groundCheckDistance;
	charBodyRotation = tPlayer->GetWorldRotation; //charBody.transform.rotation;
	rotation = charBodyRotation; //m_Rigidbody.transform.rotation;
	turnMod = 90.0f / 200.0f;
}

Player::~Player(){}

void Player::Update(float dT)
{

	// TODO
	// Handle the reset of sprint here

	combatManager->Update(dT);
}

/*
// void Move(float vert, float hori, Quaternion camRot, bool jump, bool running, bool dash, bool aiming)
// Player Movement
void Player::MoveReg(float dT, bool jump, math::float3 inputDir, math::quat camRot)
{
	Update(dT);

	if (math::length(inputDir) > 0.25f && !inAir)
	{
		math::float3 moveDir = camRot * inputDir;
		moveDir.y = 0.0f;
		moveDir = math::normalize(moveDir);
		tPlayer->FaceTo(-moveDir);

		speed += dT * kAccelerate;

		if (!isSprinting)
		{
			if (speed > moveSpeedMultiplier)
				speed = moveSpeedMultiplier;
		}
		else
		{
			if (speed > sprintSpeedMultiplier)
				speed = sprintSpeedMultiplier;
		}

		if (speed > kMaxSpeed)
			speed = kMaxSpeed;

		tPlayer->Translate(moveDir * dT * speed);

		if (!isSprinting)
		{
			aPlayer->CrossFade(1, 0.2f);
		}
		else if (isSprinting)
		{
			aPlayer->CrossFade(2, 0.3f);
		}
	}
	else if(math::length(inputDir) < 0.25f && !inAir)
	{
		speed -= dT * kDeaccelerate;
		if (speed < 0.0f) speed = 0.0f;

		tPlayer->Translate(-tPlayer->GetForwardVector() * dT * speed);\

		aPlayer->CrossFade(0, 0.2f);
	}
	else
	{
		aPlayer->CrossFade(0, 0.2f);
	}

	if (jump && !inAir)
	{
		
	}
	else if(inAir)
	{
		// Play falling animation here
		
	}
}
*/


// Move the player character
void Player::MoveReg(float dT, bool jump, math::float3 inputDir, math::quat camRot)
//void Player::MoveReg(float vert, float hori, Quaternion camRot, bool jump, bool running, bool dash, bool aiming)
{

	//*****************************************************************************************
	Update(dT);

	combatManager->SetIsMoving(false);
	moving = false;
	combatManager->SetIsSprinting(false);
	isSprinting = false;

	if (!combatManager->GetCanMove())
	{
		return;
	}

	combatManager->SetIsMoving(true);
	moving = true;
	/*
	if (!charAudio.isPlaying && m_IsGrounded)
	{
		charAudio.PlayOneShot(footsteps4);
	}*/

	//calculate initial movement direction and force
	//move = (vert * m_Rigidbody.transform.forward) + (hori * m_Rigidbody.transform.right);

	//check to see if the character is running or dashing and adjust modifier
	if (!isSprinting)
	{
		moveSpeedMultiplier = baseSpeedMultiplier;
	}
	else
	{
		moveSpeedMultiplier = sprintSpeedMultiplier;
		combatManager->SetIsSprinting(true);
	}

	CheckGroundStatus();
	//move = Vector3.ProjectOnPlane(move, m_GroundNormal);

	//m_Rigidbody.transform.RotateAround(m_Rigidbody.transform.position, m_Rigidbody.transform.up, charRotation);

	// control and velocity handling is different when grounded and airborne:
	if (isGrounded)
	{
		HandleGroundedMovement(jump);
	}
	else
	{
		HandleAirborneMovement(inputDir);
	}

	//move the character
	if (isGrounded && dT > 0)
	{
		if (math::length(inputDir) > 0.25f && !inAir)
		{
			math::float3 moveDir = camRot * inputDir;
			moveDir.y = 0.0f;
			moveDir = math::normalize(moveDir);
			tPlayer->FaceTo(-moveDir);

			speed += dT * kAccelerate;

			if (moveSpeedMultiplier > kMaxSpeed)
				moveSpeedMultiplier = kMaxSpeed;

			tPlayer->Translate(moveDir * dT * moveSpeedMultiplier);

			if (!isSprinting)
			{
				aPlayer->CrossFade(1, 0.2f);
			}
			else if (isSprinting)
			{
				aPlayer->CrossFade(2, 0.3f);
			}
		}
		else if (math::length(inputDir) < 0.25f && !inAir)
		{
			moveSpeedMultiplier -= dT * kDeaccelerate;
			if (moveSpeedMultiplier < 0.0f) moveSpeedMultiplier = 0.0f;

			tPlayer->Translate(-tPlayer->GetForwardVector() * dT * moveSpeedMultiplier); \

				aPlayer->CrossFade(0, 0.2f);
		}
		else
		{
			aPlayer->CrossFade(0, 0.2f);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Debug lines for character movement
	//Blue is m_Rigidbody forward, Red is velocity, just backwards
	//Debug.DrawLine(charPos, charPosFwd, Color.blue);
	//Debug.DrawLine(charPos, charVel, Color.red);

}//end move


// Player Movement in Aiming Mode
void Player::MoveAim(float dT, tofu::math::float3 inputDir, math::quat camRot, tofu::math::float3 camFwd)
{
	Update(dT);

	if (math::length(inputDir) > 0.25f)
	{
		math::float3 moveDir = camRot * inputDir;
		moveDir.y = 0.0f;
		moveDir = math::normalize(moveDir);

		camFwd.y = 0;
		camFwd = math::normalize(camFwd);
		tPlayer->FaceTo(-camFwd);

		speed += dT * kAccelerate;

		if (!isSprinting)
		{
			if (speed > moveSpeedMultiplier)
				speed = moveSpeedMultiplier;
		}
		else
		{
			if (speed > sprintSpeedMultiplier)
				speed = sprintSpeedMultiplier;
		}

		if (speed > kMaxSpeed)
			speed = kMaxSpeed;

		tPlayer->Translate(moveDir * dT * speed);

		aPlayer->CrossFade(1, 0.3f);
	}
	else
	{
		speed -= dT * kDeaccelerate;
		if (speed < 0.0f) speed = 0.0f;
		tPlayer->Translate(tPlayer->GetForwardVector() * dT * speed);

		aPlayer->CrossFade(0, 0.2f);
	}
}





// Handle movement on the ground
void Player::HandleGroundedMovement(bool _jump)
{
	// check whether conditions are right to allow a jump:
	if (_jump && isGrounded)
	{
		//charAudio.Stop();
		//charAudio.PlayOneShot(jumpFX);

		// jump!
		//rigidbody.velocity = math::float3(m_Rigidbody.velocity.x, jumpPower, m_Rigidbody.velocity.z);
		
		// TODO
		// Current jump mechanic
		inAir = true;
		aPlayer->CrossFade(3, 0.001f);
		pPlayer->ApplyImpulse(math::float3{ 0.0f, 4.0f, 0.0f });

		isGrounded = false;

		//jump state
		combatManager->SetIsJumping(true);
		jump = true;
		stateTimer = 0;
	}
}//end ground movement

// TODO
// Needs fixing in the Unity version and then updated here
// Handle airborne movement
void Player::HandleAirborneMovement(math::float3 inputDir)
{
	// apply extra gravity from multiplier:
	//math::float3 extraGravityForce = (Physics.gravity * gravityMultiplier) - Physics.gravity;
	//rigidbody.AddForce(extraGravityForce);
	//rigidbody.velocity = math::float3(rigidbody.velocity.x - (inputDir.z / 10), rigidbody.velocity.y, rigidbody.velocity.z + (inputDir.x / 10));

	//groundCheckDistance = m_Rigidbody.velocity.y < 0 ? origGroundCheckDistance : 0.01f;


}//end airborne movement


// check to see if player is on the ground and its status
void Player::CheckGroundStatus()
{
	math::float3 pos{ tPlayer->GetWorldPosition() };
	pos.y = pos.y + 0.1f;
	math::float3 end{ pos.x, pos.y - 0.11f, pos.z };
	RayTestResult* hitInfo;


	// Raytest to check if the player is standing on an object
	if (physics->RayTest(pos, end, hitInfo))
	{
		if (hitInfo->hitWorldNormal.y < 0.9f)
		{
			moveSpeedMultiplier = slopeSpeedMultiplier;
		}
		groundNormal = hitInfo->hitWorldNormal;

		if (!isGrounded && hasJumped)
		{
			//charAudio.Stop();
			//charAudio.PlayOneShot(landFX);
			hasJumped = false;
		}
		isGrounded = true;
	}
	else
	{
		isGrounded = false;
		groundNormal = tPlayer->GetUpVector();
	}
}// end CheckGroundStatus

// Update the Player's State
void Player::UpdateState(float dT)
{
	animationParameter = 0;

	if (stateTimer >= 0)
	{
		stateTimer += dT;
	}

	lastState = currentState;


	if ( !combatManager->UpdateState(stateTimer, dT) )
	{
		if (jump)
		{
			if (stateTimer < combatManager->GetJumpUpTime())
			{
				hasJumped = true;
				currentState = kJumpUp;
			}
			else if (stateTimer >= combatManager->GetJumpUpTime() && stateTimer < combatManager->GetJumpUpTime() + combatManager->GetJumpAirTime() )
			{
				currentState = kJumpAir;
			}
			else if (stateTimer >= combatManager->GetJumpUpTime() + combatManager->GetJumpAirTime()
				&& stateTimer < combatManager->GetJumpUpTime() + combatManager->GetJumpAirTime() + combatManager->GetJumpDownTime() )
			{
				currentState = kJumpDown;
			}
			else
			{
				stateTimer = -1;
				jump = false;
			 	combatManager->SetIsJumping(false);
			}

		}
		else if (combatManager->GetIsDodging())
		{
			if (stateTimer < combatManager->GetDodgeTime())
			{
				currentState = kDodge;
				animationParameter = combatManager->GetDodgeDirection();
			}
			else
			{
				stateTimer = -1;
			    combatManager->SetIsDodging(false);
			}
		}
		else if (combatManager->GetIsRolling())
		{
			if (stateTimer < combatManager->GetRollTime())
			{
				currentState = kRoll;
				ForceMove(combatManager->GetRollSpeed(), dT, 1);
			}
			else
			{
				stateTimer = -1;
				combatManager->SetIsRolling(false);
			}
		}
		else if (combatManager->GetIsAttacking())
		{
			if (stateTimer < combatManager->GetCurrentAttackTime())
			{
				currentState = kAttack;
				animationParameter = (int)combatManager->GetCurrentCombat();
				if (stateTimer >= combatManager->GetCurrentEffectTime() && !hasEffect)
				{
					hasEffect = true;
					combatManager->Effect();
				}
				if (combatManager->GetResetAttack())
				{
					currentState = kNoState;
					combatManager->SetResetAttack(false);
				}
			}
			else
			{
				stateTimer = -1;
				combatManager->SetIsAttacking(false);
				hasEffect = false;
				combatManager->SetComboTimer(0);
			}

		}
		else if (combatManager->GetIsAdjusting())
		{
			if (combatManager->CheckTarget())
			{
				combatManager->SetIsAdjusting(false);
				combatManager->Attack();
			}
			else
			{
				//look at target
				math::float3 target = combatManager->GetCurrentTarget()->GetPosition() - GetPosition();
				tPlayer->FaceTo(target);
				
				// TODO
				// Rotate player 
				currentState = kAdjustPosition;
				ForceMove(combatManager->GetAdjustSpeed(), dT, 1);
			}
		}
		else if (combatManager->GetIsAimming() && combatManager->GetMoveDir() > -1)
		{
			animationParameter = combatManager->GetMoveDir();
			currentState = kAimMove;
		}
		else
		{

			if (moving)
			{
				currentState = kRun;
				if (isSprinting)
				{
					animationParameter = 1;
				}
			}
			else
			{
				if (combatManager->GetInCombat())
				{
					currentState = kIdleInCombat;
				}
				else if (combatManager->GetIsAimming())
				{
					currentState = kIdleInCombat;
				}
				else
				{
					currentState = kIdleOutCombat;
				}

			}
		}
	}

	if (isDead)
	{
		currentState = kDead;
	}

	if (lastState != currentState)
	{
		gPlayer->Play(currentState, animationParameter, 0);
		aPlayer->CrossFade(0, 0.2f);
	}


}

// TODO
// Check to see if forward/right need to be negated to be right
// Force the player to move a bit
void Player::ForceMove(float speed, float dT, int direction)
{
	if (direction == 0)
	{
		tPlayer->Translate -= tPlayer->GetForwardVector() * speed * dT;
	}
	else if (direction == 1)
	{
		tPlayer->Translate += tPlayer->GetForwardVector() * speed * dT;
	}
	else if (direction == 2)
	{
		tPlayer->Translate += tPlayer->GetRightVector() * speed * dT;
	}
	else if (direction == 3)
	{
		tPlayer->Translate -= tPlayer->GetRightVector() * speed * dT;
	}
}

// Force the player to move a bit
void Player::ForceMove(float speed, float dT, math::float3 direction)
{
	tPlayer->Translate += direction * speed * dT;
}





//-------------------------------------------------------------------------------------------------
// Player Actions

// Transistion to Aiming Mode
void Player::Aim()
{
	// TODO
}

// Attack (Uses a combo system)
void Player::Attack()
{
	// TODO
}

// Sprint (move faster)
void Player::Sprint(bool _sprint)
{
	// TODO
	// Set dashing to true if cool allows
	isSprinting = _sprint;
}

// Dodge, in the current player direction
void Player::Dodge()
{
	// TODO
}

// Interact with interactable object
void Player::Interact()
{
	// TODO
}

// Combo Special move (Sword/Gun)
void Player::Special()
{
	// TODO
}

// Transistion to Vision Hack Mode
void Player::VisionHack()
{
	// TODO
}

//-------------------------------------------------------------------------------------------------
// Setters

// Set the Animation Parameter
void Player::AnimationParameter(int _animationParameter)
{
	animationParameter = _animationParameter;
}

// Set Character's current state
void Player::CurrentState(CharacterState _currentState)
{
	currentState = _currentState;
}

// Set if action has an effect
void Player::HasEffect(bool _hasEffect)
{
	hasEffect = _hasEffect;
}

// Set the character's last state
void Player::LastState(CharacterState _lastState)
{
	lastState = _lastState;
}

// Set the State Timer
void Player::StateTimer(float _stateTimer)
{
	stateTimer = _stateTimer;
}





//-------------------------------------------------------------------------------------------------
// Getters

// Is the player in air
bool Player::IsInAir()
{
	return inAir;
}

// Return Player Position
tofu::math::float3 Player::GetPosition()
{
	return tPlayer->GetLocalPosition();
}

// Return Player Forward
tofu::math::float3 Player::GetForward()
{
	return tPlayer->GetForwardVector();
}

// Return if player is grounded
bool Player::IsGrounded()
{
	return isGrounded;
}

// Return if player is dead
bool Player::IsDead()
{
	return isDead;
}

// Return if action has effect
bool Player::HasEffect()
{
	return hasEffect;
}

// Return the state timer
float Player::StateTimer()
{
	return stateTimer;
}

// Return current state
CharacterState Player::CurrentState()
{
	return currentState;
}

// Return the last state
CharacterState Player::LastState()
{
	return lastState;
}
