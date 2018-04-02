#include "Player.h"
#include <PhysicsSystem.h>
#include <RenderingComponent.h>

using namespace tofu;

Player::Player(CharacterDetails details, void* comp)
{
	CombatManagerDetails combatDetails = {};
	{
		combatDetails.moveDir = 0;
		combatDetails.inCombatDuration = 4.0f;
		combatDetails.maxShotDistance = 20.0f;
		combatDetails.minShotDistance = 2.0f;
		combatDetails.jumpUpTime = 0.8f;
		combatDetails.jumpAirTime = 0.6f;
		combatDetails.jumpDownTime = 0.2f;
		combatDetails.comboTimer = 0.0f;
		combatDetails.maxComboTime = 2.0f;
		combatDetails.dodgeTime = 0.0f;
		combatDetails.rollTime = 1.5f;
		combatDetails.rollSpeed = 60.0f;
		combatDetails.hitTime = 1.0f;
		combatDetails.hitMaxWalkSpeed = 1.0f;
		combatDetails.adjustSpeed = 2.0f;
		combatDetails.adjustMinDistance = 0.75f;
		combatDetails.adjustMaxDistance = 2.5f;
		combatDetails.adjustAgle = 5.0f;
	}

	Init(true, comp, combatDetails);

	tag = details.tag;
	{
		Entity e = Entity::Create();

		tPlayer = e.AddComponent<TransformComponent>();
		tPlayer->SetLocalPosition(details.position);
		tPlayer->SetLocalScale(details.scale);

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		{
			aPlayer = e.AddComponent<AnimationComponent>();

			AnimationStateMachine *stateMachine = aPlayer->GetStateMachine();

			// Base Layer Animations

			// Idle Animations
			AnimationState *idle = stateMachine->AddState("idle");
			idle->animationName = "idle";
			AnimationState *combat_idle = stateMachine->AddState("combat_idle");
			combat_idle->animationName = "combat_idle";


			// Movement Animations
			AnimationState *walk = stateMachine->AddState("walk");
			walk->animationName = "walk";
			AnimationState *run = stateMachine->AddState("run");
			run->animationName = "run";


			// Jump Animations
			AnimationState *jump = stateMachine->AddState("jump");
			jump->animationName = "jump";
			AnimationState *jump_up = stateMachine->AddState("jump_up");
			jump_up->animationName = "jump_up";
			AnimationState *jump_air = stateMachine->AddState("jump_air");
			jump_air->animationName = "jump_air";
			AnimationState *jump_down = stateMachine->AddState("jump_down");
			jump_down->animationName = "jump_down";


			// Roll/Dodge Animations
			AnimationState *kRoll = stateMachine->AddState("kRoll");
			kRoll->animationName = "kRoll";


			// Combat Animations
			AnimationState *kPunchJabL = stateMachine->AddState("kPunchJabL");
			kPunchJabL->animationName = "kPunchJabL";
			AnimationState *kPunchJabR = stateMachine->AddState("kPunchJabR");
			kPunchJabR->animationName = "kPunchJabR";
			AnimationState *kPunchHookL = stateMachine->AddState("kPunchHookL");
			kPunchHookL->animationName = "kPunchHookL";
			AnimationState *kPunchHookR = stateMachine->AddState("kPunchHookR");
			kPunchHookR->animationName = "kPunchHookR";
			AnimationState *kPunchUpperCutL = stateMachine->AddState("kPunchUpperCutL");
			kPunchUpperCutL->animationName = "kPunchUpperCutL";
			AnimationState *kPunchUpperCutR = stateMachine->AddState("kPunchUpperCutR");
			kPunchUpperCutR->animationName = "kPunchUpperCutR";
			AnimationState *kKickAxeKick = stateMachine->AddState("kKickAxeKick");
			kKickAxeKick->animationName = "kKickAxeKick";
			AnimationState *kKickHorseKick = stateMachine->AddState("kKickHorseKick");
			kKickHorseKick->animationName = "kKickHorseKick";
			AnimationState *kKickStraightMidR = stateMachine->AddState("kKickStraightMidR");
			kKickStraightMidR->animationName = "kKickStraightMidR";
			AnimationState *kKickKnee = stateMachine->AddState("kKickKnee");
			kKickKnee->animationName = "kKickKnee";
		}

		// Upper Layer Animations
		{
			AnimationLayer *upperLayer = aPlayer->AddLayer("Upper", 1.0f, kAET_Override);

			// TODO This is causing memory leaks
			upperLayer->selectedJoints = new std::vector<uint16_t>();

			for (int i = 3; i <= 55; i++) 
			{
				upperLayer->selectedJoints->push_back(i);
			}

			AnimationStateMachine *stateMachine = upperLayer->GetStateMachine();

			// Idle Animations
			AnimationState *idle = stateMachine->AddState("idle");
			idle->animationName = "idle";
			AnimationState *combat_idle = stateMachine->AddState("combat_idle");
			combat_idle->animationName = "combat_idle";


			// Movement Animations
			AnimationState *walk = stateMachine->AddState("walk");
			walk->animationName = "walk";
			AnimationState *run = stateMachine->AddState("run");
			run->animationName = "run";


			// Jump Animations
			AnimationState *jump = stateMachine->AddState("jump");
			jump->animationName = "jump";
			AnimationState *jump_up = stateMachine->AddState("jump_up");
			jump_up->animationName = "jump_up";
			AnimationState *jump_air = stateMachine->AddState("jump_air");
			jump_air->animationName = "jump_air";
			AnimationState *jump_down = stateMachine->AddState("jump_down");
			jump_down->animationName = "jump_down";


			// Roll/Dodge Animations
			AnimationState *kRoll = stateMachine->AddState("kRoll");
			kRoll->animationName = "kRoll";


			// Combat Animations
			AnimationState *kPunchJabL = stateMachine->AddState("kPunchJabL");
			kPunchJabL->animationName = "kPunchJabL";
			AnimationState *kPunchJabR = stateMachine->AddState("kPunchJabR");
			kPunchJabR->animationName = "kPunchJabR";
			AnimationState *kPunchHookL = stateMachine->AddState("kPunchHookL");
			kPunchHookL->animationName = "kPunchHookL";
			AnimationState *kPunchHookR = stateMachine->AddState("kPunchHookR");
			kPunchHookR->animationName = "kPunchHookR";
			AnimationState *kPunchUpperCutL = stateMachine->AddState("kPunchUpperCutL");
			kPunchUpperCutL->animationName = "kPunchUpperCutL";
			AnimationState *kPunchUpperCutR = stateMachine->AddState("kPunchUpperCutR");
			kPunchUpperCutR->animationName = "kPunchUpperCutR";
			AnimationState *kKickAxeKick = stateMachine->AddState("kKickAxeKick");
			kKickAxeKick->animationName = "kKickAxeKick";
			AnimationState *kKickHorseKick = stateMachine->AddState("kKickHorseKick");
			kKickHorseKick->animationName = "kKickHorseKick";
			AnimationState *kKickStraightMidR = stateMachine->AddState("kKickStraightMidR");
			kKickStraightMidR->animationName = "kKickStraightMidR";
			AnimationState *kKickKnee = stateMachine->AddState("kKickKnee");
			kKickKnee->animationName = "kKickKnee";
		}

		// Material and Model
		{			
			const char* name = details.modelName.c_str();
			Model* model = RenderingSystem::instance()->CreateModel(name);
			Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialDeferredGeometryOpaqueSkinned);
			name = details.diffuseName.c_str();
			TextureHandle diffuse = RenderingSystem::instance()->CreateTexture(name);
			name = details.normalMapName.c_str();
			TextureHandle normalMap = RenderingSystem::instance()->CreateTexture(name);
			
			material->SetTexture(diffuse);
			material->SetNormalMap(normalMap);

			r->SetMaterial(material);
			r->SetModel(model);
		}

		// Physics
		pPlayer = e.AddComponent<PhysicsComponent>();

		pPlayer->LockRotation(true, true, true);
		pPlayer->SetCapsuleCollider(details.capsuleColliderSize.x, details.capsuleColliderSize.y);
		pPlayer->SetColliderOrigin(details.colliderOrigin);
		pPlayer->SetGravity(math::float3{});

		lastMoveDir = {};
		velocity = {};
		charRot = {};

		SetComponents(tPlayer, pPlayer, aPlayer);
	}
	
	baseSpeedMultiplier = details.walkSpeed;
	moveSpeedMultiplier = baseSpeedMultiplier;
	sprintSpeedMultiplier = details.sprintSpeed;
	airborneSpeedMultiplier = moveSpeedMultiplier * 0.5f;
	
	jumpPower = details.jumpPower;

	rollDodgeCost = details.rollDodgeCost;

	stateTimer = 0;
	
	physics = tofu::PhysicsSystem::instance();

	gCharacter->SetAnimComp(aPlayer);


	//rigidbody = GetComponent<Rigidbody>();
	//rigidbody.constraints = RigidbodyConstraints.FreezeRotationX | RigidbodyConstraints.FreezeRotationY | RigidbodyConstraints.FreezeRotationZ;
	origGroundCheckDistance = groundCheckDistance;
	charBodyRotation = tPlayer->GetWorldRotation(); //charBody.transform.rotation;
	rotation = charBodyRotation; //m_Rigidbody.transform.rotation;
	turnMod = 90.0f / 200.0f;

	attackButtonDown = false;
	specialButtonDown = false;
	isSprinting = false;
	isRolling = false;
	attackButtonTimer = 0.0f;
	specialButtonTimer = 0.0f;
	minHoldTime = 0.5f;
	maxHoldTime = 1.5f; // Approximatelly measured in seconds

	gun = new Gun();
}

Player::~Player()
{
	delete gun;
}

void Player::Update(float dT)
{
	Character::Update(dT);
	UpdateState(dT);
	//combatManager->Update(dT);

	// If attack button is still pressed, add to the timer
	if (attackButtonDown)
	{
		attackButtonTimer += dT;
	}

	// If player has held the button beyond the max attack hold time auto trigger the attack
	if(attackButtonTimer >= maxHoldTime && attackButtonDown)
	{
		combatManager->SpecialCombat();
		attackButtonDown = false;
	}
	if (specialButtonTimer >= maxHoldTime && specialButtonDown)
	{
		combatManager->SwordSpecialCombat();
		specialButtonDown = false;
	}
}

void Player::FixedUpdate(float fDT)
{
	if (isRolling)
	{
		CheckGroundStatus();

		if (isGrounded)
		{
			HandleGroundedMovement(lastMoveDir, true, false, fDT);
		}
		else if (!isGrounded)
		{
			//HandleAirborneMovement(lastMove, dT);
			HandleAirborneMovement(lastMoveDir, true, fDT);
		}

		pPlayer->SetVelocity(velocity);
	}

}

// Move the player character
void Player::MoveReg(float dT, bool _jump, math::float3 inputDir, math::quat camRot)
//void Player::MoveReg(float vert, float hori, Quaternion camRot, bool jump, bool running, bool dash, bool aiming)
{
	float vert = 0.0f;
	float hori = 0.0f;
	// Linear interpolation of movement
	if (!isSprinting)
	{
		if (inputDir.z > 0.0f)
		{
			vert = math::mix(0.3f, 1.0f, vertLerp);

			if (vertLerp < 1.0f)
			{
				vertLerp += lerpMod;
			}
		}
		else if (inputDir.z < 0.0f)
		{
			vert = -math::mix(0.3f, 1.0f, vertLerp);

			if (vertLerp < 1.0f)
			{
				vertLerp += lerpMod;
			}
		}
		else
		{
			vertLerp = 0.35f;
			vert = 0.0f;
		}
		inputDir.z = vert;

		if (inputDir.x > 0.0f)
		{
			hori = math::mix(0.3f, 1.0f, horiLerp);

			if (horiLerp < 1.0f)
			{
				horiLerp += lerpMod;
			}
		}
		else if (inputDir.x < 0.0f)
		{
			hori = -math::mix(0.3f, 1.0f, horiLerp);

			if (horiLerp < 1.0f)
			{
				horiLerp += lerpMod;
			}
		}
		else
		{
			horiLerp = 0.35f;
			hori = 0.0f;
		}

		inputDir.x = hori;
	}


	lastVelocity = pPlayer->GetVelocity();
	charRot = pPlayer->GetRotation();

	combatManager->SetIsMoving(false);
	moving = false;

	if (!combatManager->GetCanMove())
	{
		return;
	}

	bool hasInputDir = math::length(inputDir) > 0.25f;
	//bool hasInputDir = math::length(inputDir) > 0.01f;
	math::float3 moveDir = { 0,0,0 };

	velocity = lastVelocity;

	//combatManager->SetIsSprinting(false);
	//isSprinting = false;

	/*
	if (!charAudio.isPlaying && m_IsGrounded)
	{
		charAudio.PlayOneShot(footsteps4);
	}*/

	CheckGroundStatus();

	math::float3 fwd{ 0, 0, 1 }; //???
	if (hasInputDir)
	{
		moveDir = camRot * inputDir;
		moveDir.y = 0.0f;
		moveDir = math::normalize(moveDir);

		if (isGrounded)
		{
			combatManager->SetIsMoving(true);
			moving = true;

			if (!isSprinting)
			{
				moveSpeedMultiplier = baseSpeedMultiplier;
			}
			else
			{
				moveSpeedMultiplier = sprintSpeedMultiplier;
				combatManager->SetIsSprinting(true);
			}

			if (moveSpeedMultiplier > kMaxSpeed)
				moveSpeedMultiplier = kMaxSpeed;

			// Character's facing direction
			{
				math::float3 faceDir = -moveDir;
				float cosTheta = math::dot(faceDir, fwd);
				if (cosTheta >= 1.0 - FLT_EPSILON)
				{
					charRot = math::quat();
				}
				else if (-cosTheta >= 1.0 - FLT_EPSILON)
				{
					charRot = { 0, 0, 1, 0 };
				}
				else
				{
					float angle = math::acos(cosTheta);
					math::float3 axis = math::normalize(math::cross(fwd, faceDir));
					charRot = math::angleAxis(angle, axis);
				}
				pPlayer->SetRotation(charRot);
			}
		}	
	}

	
	// control and velocity handling is different when grounded and airborne:
	if (isGrounded)
	{
		HandleGroundedMovement(moveDir, hasInputDir, _jump, dT);
	}
	else if (!isGrounded)
	{
		//HandleAirborneMovement(lastMove, dT);
		HandleAirborneMovement(moveDir, hasInputDir, dT);
	}

	if (hasJumped && isGrounded)
	{
		velocity.y = jumpPower;
	}

	pPlayer->SetVelocity(velocity);

	if (moveDir.x != 0 || moveDir.z != 0)
	{
		lastMoveDir = moveDir;
	}

	//---------------------------------------------------------------------------------------
	/*
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
	*/
	//------------------------------------------------------------------------------------------
	//Update(dT);
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Debug lines for character movement
	//Blue is m_Rigidbody forward, Red is velocity, just backwards
	//Debug.DrawLine(charPos, charPosFwd, Color.blue);
	//Debug.DrawLine(charPos, charVel, Color.red);

}//end move

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
				currentState = kJumpingPrepare;
			}
			else if (stateTimer >= combatManager->GetJumpUpTime() && stateTimer < combatManager->GetJumpUpTime() + combatManager->GetJumpAirTime() )
			{
				currentState = kJumpUp;
				//currentState = kJumpAir;
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
		else if (combatManager->GetIsRolling())
		{
			if (stateTimer < combatManager->GetRollTime())
			{
				currentState = kRoll;
			}
			else if (stateTimer > combatManager->GetRollTime() / 2 && stateTimer < combatManager->GetRollTime())
			{
				ForceMove(combatManager->GetRollSpeed(), dT, 1);
			}
			else
			{
				stateTimer = -1;
				combatManager->SetIsRolling(false);
				isRolling = false;
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
				currentState = kWalk;
				if (isSprinting)
				{
					currentState = kRun;
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
		gCharacter->Play(currentState, animationParameter, 0);
		//aPlayer->CrossFade(0, 0.2f);
	}


}


//-------------------------------------------------------------------------------------------------
// Player Actions

// Attack (Uses a combo system)
void Player::Attack(bool down, float dT)
{
	// TODO
	if (!isAiming)
	{
		// Primary Attack
		// Attack button pressed
		if (!attackButtonDown && down) //Button 2
		{
			attackButtonDown = true;
			attackButtonTimer = 0;
		}
		
		// If attack button is released and timer is less than a hold time, use basic attack
		if (attackButtonDown && !down && attackButtonTimer <= minHoldTime)
		{
			combatManager->BasicCombo();
			attackButtonDown = false;
		}

		// If attack button is released
		if ((!down && attackButtonDown && attackButtonTimer > minHoldTime))
		{
			combatManager->SpecialCombat();
			attackButtonDown = false;
		}
	}
	else if (isAiming)	// Gun Attack
	{
		if(!gun->GetIsActive())
		{
			gun->SetIsActive(true);
		}
		if (!attackButtonDown && down) // TODO Add hook into player energy pool: playerCharacter.SpecialBar > 33.9999f
		{
			attackButtonDown = true;
			attackButtonTimer = 0;
			combatManager->GunShot();
		}
		if (attackButtonDown && !down)
		{
			attackButtonDown = false;
		}
	}
}

void Player::Die()
{

}

// Dodge, in the current player direction
void Player::Dodge(tofu::math::float3 inputDir)
{
    // If can roll
	if(combatManager->GetCanRoll())
    {
		isRolling = true;
		combatManager->Roll();
        // Remove stamina
        //playerCharacter.UseStamina(rollDodgeCost);
    }
}

// Interact with interactable object
void Player::Interact()
{
	// TODO
	// Probably a dead feature
}

// Combo Special move (Sword)
void Player::Special(bool down, float dT)
{
	// TODO
	if (!isAiming)
	{
		// Special (Sword) Attack
		if (specialButtonDown)
		{
			specialButtonTimer += dT;
		}
		if (specialButtonDown && down ) // TODO Add energy hook: playerCharacter.SpecialBar > 24.9999f
		{
			specialButtonDown = true;
			specialButtonTimer = 0;
		}
		// Button Press Attack
		if (specialButtonDown && !down && specialButtonTimer <= minHoldTime)	// TODO Add energy hook: playerCharacter.SpecialBar > 24.9999f
		{
			combatManager->SwordCombo();
			specialButtonDown = false;
		}
		// Button Hold Attack
		/*
		if (((CrossPlatformInputManager.GetButtonUp("Sword") && specialButtonDown && specialButtonTimer > minHoldTime)
		|| (specialButtonTimer >= maxHoldTime && specialButtonDown)) && playerCharacter.SpecialBar > 49.9999f)
		*/
		if ( ((specialButtonDown && !down && specialButtonTimer > minHoldTime)	// TODO Add energy hook: playerCharacter.SpecialBar > 49.9999f
			|| (specialButtonTimer >= maxHoldTime && specialButtonDown)) )
		{
			combatManager->SwordSpecialCombat();
			specialButtonDown = false;
		}
	}
}


// Transistion to Vision Hack Mode
void Player::VisionHack()
{
	// TODO
	/*
	if (CrossPlatformInputManager.GetButtonDown("Hack")) //Button 4
        {
            //if (visionHackCDTimer == visionHackCD)
            if (playerCharacter.SpecialBar > 20 && !m_hacking)
            {
                Debug.Log("Vision Hack");
                m_hacking = true;
                StartVisionHack();
            }
            else {
                Debug.Log("Cooling Down");
            }
            
        }
        else
        {
            m_hacking = false;
        }
	*/
}

//-------------------------------------------------------------------------------------------------
// Setters







//-------------------------------------------------------------------------------------------------
// Getters




