#include "Player.h"
#include <PhysicsSystem.h>
#include <RenderingComponent.h>

using namespace tofu;

Player::Player(CharacterDetails details, void* comp)
{
	CombatManagerDetails combatDetails = {};
	combatDetails.moveDir = 0;
	combatDetails.inCombatDuration = 4.0f;
	combatDetails.maxShotDistance = 20.0f;
	combatDetails.minShotDistance = 2.0f;
	combatDetails.jumpUpTime = 0.5f;
	combatDetails.jumpAirTime = 0.3f;
	combatDetails.jumpDownTime = 0.35f;
	combatDetails.comboTimer = 0.0f;
	combatDetails.maxComboTime = 2.0f;
	combatDetails.dodgeTime = 0.0f;
	combatDetails.rollTime = 0.5f;
	combatDetails.rollSpeed = 3.0f;
	combatDetails.hitTime = 1.0f;
	combatDetails.hitMaxWalkSpeed = 1.0f;
	combatDetails.adjustSpeed = 2.0f;
	combatDetails.adjustMinDistance = 0.75f;
	combatDetails.adjustMaxDistance = 2.5f;
	combatDetails.adjustAgle = 5.0f;

	Init(true, comp, combatDetails);

	tag = details.tag;
	{
		Entity e = Entity::Create();

		tPlayer = e.AddComponent<TransformComponent>();
		tPlayer->SetLocalPosition(details.position);
		tPlayer->SetLocalScale(details.scale);

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
		pPlayer->SetCapsuleCollider(details.capsuleColliderSize.x, details.capsuleColliderSize.y);
		pPlayer->SetColliderOrigin(details.colliderOrigin);


		SetComponents(tPlayer, pPlayer, aPlayer);
	}

	baseSpeedMultiplier = details.walkSpeed;
	moveSpeedMultiplier = baseSpeedMultiplier;
	sprintSpeedMultiplier = details.sprintSpeed;

	jumpPower = details.jumpPower;

	stateTimer = 0;
	
	physics = tofu::PhysicsSystem::instance();

	gPlayer = new GameplayAnimationMachine(aPlayer, combatManager);


	//rigidbody = GetComponent<Rigidbody>();
	//rigidbody.constraints = RigidbodyConstraints.FreezeRotationX | RigidbodyConstraints.FreezeRotationY | RigidbodyConstraints.FreezeRotationZ;
	origGroundCheckDistance = groundCheckDistance;
	charBodyRotation = tPlayer->GetWorldRotation(); //charBody.transform.rotation;
	rotation = charBodyRotation; //m_Rigidbody.transform.rotation;
	turnMod = 90.0f / 200.0f;

	move = { 0.0f, 0.0f, 0.0f };
	lastMove = { 0.0f, 0.0f, 0.0f };
}

Player::~Player(){}

void Player::Update(float dT)
{
	Character::Update(dT);
	UpdateState(dT);
	//combatManager->Update(dT);
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
	//Update(dT);

	move = { 0.0f, 0.0f, 0.0f };

	combatManager->SetIsMoving(false);
	moving = false;
	combatManager->SetIsSprinting(false);
	isSprinting = false;

	if (!combatManager->GetCanMove())
	{
		return;
	}


	if (isGrounded && jump)
	{
		HandleGroundedMovement(jump, lastMove, dT);
	}

	/*
	if (!charAudio.isPlaying && m_IsGrounded)
	{
		charAudio.PlayOneShot(footsteps4);
	}*/

	//calculate initial movement direction and force
	//move = (vert * m_Rigidbody.transform.forward) + (hori * m_Rigidbody.transform.right);

	

	CheckGroundStatus();
	//move = Vector3.ProjectOnPlane(move, m_GroundNormal);

	//m_Rigidbody.transform.RotateAround(m_Rigidbody.transform.position, m_Rigidbody.transform.up, charRotation);

	//move the character
	if (isGrounded && dT > 0)
	{
		if (math::length(inputDir) > 0.25f)
		{
			combatManager->SetIsMoving(true);
			moving = true;
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

			math::float3 moveDir = camRot * inputDir;
			moveDir.y = 0.0f;
			moveDir = math::normalize(moveDir);
			tPlayer->FaceTo(-moveDir);

			speed += dT * kAccelerate;

			if (moveSpeedMultiplier > kMaxSpeed)
				moveSpeedMultiplier = kMaxSpeed;

			move = moveDir * dT * moveSpeedMultiplier;

			//tPlayer->Translate(move);
			//pPlayer->SetVelocity(move);

			if (!isSprinting)
			{
				//aPlayer->CrossFade(1, 0.2f);
			}
			else if (isSprinting)
			{
				//aPlayer->CrossFade(2, 0.3f);
			}
		}
		else if (math::length(inputDir) < 0.25f && dT > 0)
		{
			//combatManager->SetIsMoving(true);
			//moving = true;
			moveSpeedMultiplier -= dT * kDeaccelerate;
			if (moveSpeedMultiplier < 0.0f) moveSpeedMultiplier = 0.0f;

			move = -tPlayer->GetForwardVector() * dT * moveSpeedMultiplier;

			//tPlayer->Translate(move);
			//pPlayer->SetVelocity(move);

			//aPlayer->CrossFade(0, 0.2f);
		}
	}// End if Grounded
	
	else
	{
		moveSpeedMultiplier -= dT * kAirDeaccelerate;
		if (moveSpeedMultiplier < 0.0f) moveSpeedMultiplier = 0.0f;

		move = -tPlayer->GetForwardVector() * dT * moveSpeedMultiplier;
	}


	Update(dT);
	

	 // control and velocity handling is different when grounded and airborne:
	if (isGrounded && !jump)
	{
		HandleGroundedMovement(jump, move, dT);
	}
	else
	{
		HandleAirborneMovement(lastMove, dT);
	}

	lastMove = move;

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

		//tPlayer->Translate(moveDir * dT * speed);

		//aPlayer->CrossFade(1, 0.3f);
	}
	else
	{
		speed -= dT * kDeaccelerate;
		if (speed < 0.0f) speed = 0.0f;
		//tPlayer->Translate(tPlayer->GetForwardVector() * dT * speed);

		//aPlayer->CrossFade(0, 0.2f);
	}
}






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
		//aPlayer->CrossFade(0, 0.2f);
	}


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

void Player::Die()
{

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

// Combo Special move (Sword)
void Player::Special()
{
	// TODO
	// Use special attack
}

// Transistion to Vision Hack Mode
void Player::VisionHack()
{
	// TODO
}

//-------------------------------------------------------------------------------------------------
// Setters







//-------------------------------------------------------------------------------------------------
// Getters




