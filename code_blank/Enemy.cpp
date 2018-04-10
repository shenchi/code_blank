#include "Enemy.h"
#include <PhysicsSystem.h>
#include <RenderingComponent.h>

using namespace tofu;

Enemy::Enemy(CharacterDetails details, void* model, void* material)
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

	Init(false, nullptr, combatDetails);

	tag = details.tag;

	// TODO Make a new action selector here
	//s_action = GetComponent<ActionSelector>();

	//seekTarget = playerTarget = GameObject.FindGameObjectWithTag("Player").transform;
	seekTarget = {};


	customNavigationAgent = NavigationAgent();

	//m_combat.SetChar(this);

	// Set the destination for the NavMesh.
	if (tofu::math::float3{} != seekTarget)
	{
		customNavigationAgent.SetDestination(seekTarget);
	}

	{
		Entity e = Entity::Create();

		tEnemy = e.AddComponent<TransformComponent>();
		tEnemy->SetLocalPosition(details.position);
		tEnemy->SetLocalScale(math::float3{ 0.01f, 0.01f, 0.01f });

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		//Model* model = RenderingSystem::instance()->CreateModel("assets/archer.model");


		{
			aEnemy = e.AddComponent<AnimationComponent>();

			AnimationStateMachine *stateMachine = aEnemy->GetStateMachine();

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
			AnimationState *kRoll = stateMachine->AddState("kRoll", false);
			kRoll->animationName = "kRoll";
			kRoll->playbackSpeed = 1.08f;


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
			AnimationState *kSwordR = stateMachine->AddState("kSwordR");
			kSwordR->animationName = "kSwordR";
			AnimationState *kSwordR2 = stateMachine->AddState("kSwordR2");
			kSwordR2->animationName = "kSwordR2";
			AnimationState *kSwordCombo = stateMachine->AddState("kSwordCombo");
			kSwordCombo->animationName = "kSwordCombo";

			AnimationState *kDeath = stateMachine->AddState("kDeath", false);
			kDeath->animationName = "kDeath";
		}

		// Upper Layer Animations
		{
			AnimationLayer *upperLayer = aEnemy->AddLayer("Upper", 1.0f, kAET_Override);

			// TODO This is causing memory leaks
			//upperLayer->selectedJoints = new std::vector<uint16_t>();

			for (int i = 3; i <= 55; i++)
			{
				upperLayer->selectedJoints.push_back(i);
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
			AnimationState *kRoll = stateMachine->AddState("kRoll", false);
			kRoll->animationName = "kRoll";
			kRoll->playbackSpeed = 1.08f;


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
			AnimationState *kSwordR = stateMachine->AddState("kSwordR");
			kSwordR->animationName = "kSwordR";
			AnimationState *kSwordR2 = stateMachine->AddState("kSwordR2");
			kSwordR2->animationName = "kSwordR2";
			AnimationState *kSwordCombo = stateMachine->AddState("kSwordCombo");
			kSwordCombo->animationName = "kSwordCombo";

			AnimationState *kDeath = stateMachine->AddState("kDeath", false);
			kDeath->animationName = "kDeath";
		}

		/*Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialDeferredGeometryOpaqueSkinned);
		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/archer_0.texture");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/archer_1.texture");

		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);*/

		r->SetMaterial(static_cast<Material*>(material));
		r->SetModel(static_cast<Model*>(model));

		pEnemy = e.AddComponent<PhysicsComponent>();

		pEnemy->LockRotation(true, true, true);
		pEnemy->SetCapsuleCollider(details.capsuleColliderSize.x, details.capsuleColliderSize.y);
		pEnemy->SetColliderOrigin(details.colliderOrigin);
		pEnemy->SetGravity(math::float3{});

		SetComponents(tEnemy, pEnemy, aEnemy);
	}

	walkSpeed = 2.0f;
	dashSpeed = 4.0f;

	jumpPower = 4.0f;

	stateTimer = 0;

	physics = tofu::PhysicsSystem::instance();

	//combatManager = new CombatManager(true, comp, this);

	//gEnemy = new GameplayAnimationMachine(aEnemy, combatManager);
	gCharacter->SetAnimComp(aEnemy);

	//rigidbody = GetComponent<Rigidbody>();
	//rigidbody.constraints = RigidbodyConstraints.FreezeRotationX | RigidbodyConstraints.FreezeRotationY | RigidbodyConstraints.FreezeRotationZ;
	origGroundCheckDistance = groundCheckDistance;
	charBodyRotation = tEnemy->GetWorldRotation(); //charBody.transform.rotation;
	rotation = charBodyRotation; //m_Rigidbody.transform.rotation;
	turnMod = 90.0f / 200.0f;

	customNavigationAgent.Init(this, details.position);
}

Enemy::~Enemy() {}


void Enemy::FixedUpdate(float fDT)
{}


void Enemy::Update(float dT)
{
	// Temp
	// -------------------------------------------------------------------
	Character::Update(dT);
	UpdateState(dT);
	// -------------------------------------------------------------------

	{// May cause performance hit
		tofu::math::quat rotB = charBody.transform.rotation;
		rotB.x = 0.0f;
		rotB.z = 0.0f;
		charBody.transform.rotation = rotB;
	}

	if (GameObject.FindGameObjectWithTag("Ghost") != null)
	{
		ghostTarget = GameObject.FindGameObjectWithTag("Ghost").transform;
		seekTarget = ghostTarget;
	}
	else if (GameObject.FindGameObjectWithTag("GhostRecord") != null)
	{
		seekTarget = null;
	}
	else
	{
		seekTarget = playerTarget;
	}
	if (null == seekTarget)
	{
		// Play the Idle Animation.
		return;
	}

	if (this.m_combat.IsTurning && (null != m_combat.CurrentTarget))
	{
		Vector3 direc = this.m_combat.CurrentTarget.transform.position - transform.position;
		Quaternion rot = Quaternion.LookRotation(direc, transform.TransformDirection(Vector3.up));

		// TODO: The angle calculation seem a bit off.
		float angle = Quaternion.Angle(transform.rotation, new Quaternion(0, rot.y, 0, rot.w));
		if (angle < 20) { this.m_combat.IsTurning = false; }
		else
		{
			transform.rotation = Quaternion.RotateTowards(transform.rotation, new Quaternion(0, rot.y, 0, rot.w), turnSpeed * Time.deltaTime);
		}
	}

	if (this.m_combat.IsAdjusting)
	{
		// ??? ????????, ??????? ??????? ??????. ????.
		// Well... Adjust.. and adjust only.. Do not move do not look to perform the next action..
		float distanceToTarget = Vector3.Distance(this.transform.position, this.m_combat.CurrentTarget.transform.position);
		if (distanceToTarget > this.m_combat.GetAdjustMaxDistance())
		{
			ForceMove(1.0f, 1);
			// TODO: Check if this return affects something else? 
			return;
		}
		else if (distanceToTarget < this.m_combat.GetAdjustMinDistance())
		{
			// Going Back is 0 and not negative 1.. Welp
			ForceMove(1.0f, 0);
			// TODO: Check if this affects something else? 
			return;
		}
		else if (!(distanceToTarget > this.m_combat.GetAdjustMaxDistance() || distanceToTarget < this.m_combat.GetAdjustMinDistance()))
		{
			this.m_combat.IsAdjusting = false;
		}

		// Also Check if the adjusting is done..
	}



	if (ghostTarget != null && Vector3.Distance(transform.position, ghostTarget.position) <= maxSensoryRadius)
	{
		if (Vector3.Distance(transform.position, seekTarget.position) <= this.m_combat.GetAdjustMaxDistance())
		{
			this.customNavigationAgent.SetIsStopped(true);
			this.m_combat.IsMoving = false;

			if (null == this.m_combat.CurrentTarget)
			{
				this.m_combat.CurrentTarget = seekTarget.gameObject.GetComponent<Character>();
			}

			if (timer > m_combat.TimeBetweenAttacks)
			{
				// Only call this when we aren't stunned..
				// ??? ????????? ???? ??????.
				if (!this.m_combat.IsHit)
				{
					this.s_action.selectNextOption();
					timer = 0;
				}
			}
		}
		else
		{
			Vector3 targetPos = seekTarget.position;
			this.customNavigationAgent.SetIsStopped(false);

			// If the player moves, and the distance b/w your target and their position is >= .. , Recalculate the Path.
			if (Vector3.Distance(this.transform.position, seekTarget.position) >= this.m_combat.GetAdjustMaxDistance())
			{
				customNavigationAgent.SetDestination(seekTarget.position, seekTarget.gameObject.layer);
			}

			// Play the Animation here            
			this.m_combat.IsMoving = true;
		}
	}
	else if (Vector3.Distance(transform.position, seekTarget.position) <= maxSensoryRadius)
	{
		if (Vector3.Distance(transform.position, seekTarget.position) <= this.m_combat.GetAdjustMaxDistance())
		{

			this.customNavigationAgent.SetIsStopped(true);
			this.m_combat.IsMoving = false;

			if (null == this.m_combat.CurrentTarget)
			{
				this.m_combat.CurrentTarget = seekTarget.gameObject.GetComponent<Character>();
			}

			if (timer > m_combat.TimeBetweenAttacks)
			{
				// TODO: Use the Action Selector here. Select an Item and then, reduce the preference.
				this.s_action.selectNextOption();
				timer = 0;
			}
		}
		else
		{
			this.customNavigationAgent.SetIsStopped(false);
			// If the player moves, and the distance b/w yourself and their position is >= .. , Recalculate the Path.
			if (Vector3.Distance(this.transform.position, seekTarget.position) >= this.m_combat.GetAdjustMaxDistance())
			{
				customNavigationAgent.SetDestination(seekTarget.position, seekTarget.gameObject.layer);
			}
			// Play the Animation here            
			this.m_combat.IsMoving = true;
		}
	}
	else
	{
		// TODO: Play IDLE Animaiton Here.
		this.customNavigationAgent.SetIsStopped(true);
		this.m_combat.IsMoving = false;
	}

	// Update the Moving State for animating..
	this.m_moving = !(customNavigationAgent.isStopped);
	timer += Time.deltaTime;
	UpdateState();



}

void Enemy::UpdateState(float dT)
{

	animationParameter = 0;

	if (stateTimer >= 0)
	{
		stateTimer += dT;
	}

	lastState = currentState;


	if (!combatManager->UpdateState(stateTimer, dT) && !isDead)
	{
		if (jump)
		{
			if (stateTimer < combatManager->GetJumpUpTime())
			{
				hasJumped = true;
				currentState = kJumpingPrepare;
			}
			else if (stateTimer >= combatManager->GetJumpUpTime() && stateTimer < combatManager->GetJumpUpTime() + combatManager->GetJumpAirTime())
			{
				currentState = kJumpAir;
			}
			else if (stateTimer >= combatManager->GetJumpUpTime() + combatManager->GetJumpAirTime()
				&& stateTimer < combatManager->GetJumpUpTime() + combatManager->GetJumpAirTime() + combatManager->GetJumpDownTime())
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
			else if (stateTimer > combatManager->GetRollTime() *.75 && stateTimer < combatManager->GetRollTime())
			{
				ForceMove(400.0f, dT);
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
			float currentAttackTime = combatManager->GetCurrentAttackTime();
			if (stateTimer < currentAttackTime)
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
				if (animationParameter == 12)
				{
					if (stateTimer > currentAttackTime * 0.99f)
					{
						math::float3 pos = pEnemy->GetPosition();
						math::float3 fwd = tCharacter->GetForwardVector() * -1.0f;
						fwd = math::normalize(fwd);
						fwd = fwd * dT * 70.0f;
						pEnemy->SetPosition(pos + fwd);
					}
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
				tEnemy->FaceTo(target);

				// TODO
				// Rotate player 
				currentState = kAdjustPosition;
				ForceMove(combatManager->GetAdjustSpeed(), dT, 1);
			}
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

	else if (isDead)
	{

		if (currentState != kDead && currentState != kNoState)
		{
			currentState = kDead;
			stateTimer = 0;
		}
		else if (stateTimer > deathTimer)
		{
			currentState = kNoState;
		}
	}

	if (lastState != currentState)
	{
		gCharacter->Play(currentState, GetAnimationDuration(lastState), 0);
	}
}





// TODO
// Change as needed, this is really only a temp function for testing
// Enemy Movement
void Enemy::MoveEnemy(float dT, bool jump, math::float3 inputDir)
{
	Update(dT);

	if (math::length(inputDir) > 0.25f)
	{
		//math::float3 moveDir = rot * inputDir;
		math::float3 moveDir = inputDir;
		moveDir.y = 0.0f;
		moveDir = math::normalize(moveDir);
		tEnemy->FaceTo(moveDir);
		
		speed += dT * kAccelerate;

		if (!isDashing)
		{
			if (speed > walkSpeed)
				speed = walkSpeed;
		}
		else
		{
			if (speed > dashSpeed)
				speed = dashSpeed;
		}

		if (speed > kMaxSpeed)
			speed = kMaxSpeed;

		tEnemy->Translate(moveDir * dT * speed);

		//aEnemy->CrossFade(1, 0.3f);
		aEnemy->CrossFade("Walk", 0.3f, 0);
	}
	else
	{
		speed -= dT * kDeaccelerate;
		if (speed < 0.0f) speed = 0.0f;
		tEnemy->Translate(tEnemy->GetForwardVector() * dT * speed);

		aEnemy->CrossFade("Idle", 0.2f, 0);
		//aEnemy->CrossFade(0, 0.2f);
	}

	if (jump && !inAir)
	{
		//pEnemy->ApplyImpulse(math::float3{ 0.0f, 2.0f, 0.0f });
	}
}



//-------------------------------------------------------------------------------------------------
// Enemy Actions

// Aim at Player
void Enemy::Aim()
{

}


// Attack (Uses a combo system)
void Enemy::Attack()
{
	// TODO
}

// Kill off the enmey by turning it off and hiding it
void Enemy::Die()
{

}


//-------------------------------------------------------------------------------------------------
// Setters



//-------------------------------------------------------------------------------------------------
// Getters

float Enemy::GetMaxSensoryRadius()
{
	return maxSensoryRadius;
}