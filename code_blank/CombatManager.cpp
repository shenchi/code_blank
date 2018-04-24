#include "CombatManager.h"
//#include "Character.h"

using namespace tofu;

CombatManager::CombatManager(bool _isPlayer, void* _companion, void* owner, CombatManagerDetails details)
{
	isPlayer = _isPlayer;


	moveDir = details.moveDir;
	inCombatDuration = details.inCombatDuration;
	maxShotDistance = details.maxShotDistance;
	minShotDistance = details.minShotDistance;
	jumpUpTime = details.jumpUpTime;
	jumpAirTime = details.jumpAirTime;
	jumpDownTime = details.jumpDownTime;
	comboTimer = details.comboTimer;
	maxComboTime = details.maxComboTime;
	dodgeTime = details.dodgeTime;
	rollTime = details.rollTime;
	rollSpeed = details.rollSpeed;
	hitTime = details.hitTime;
	hitMaxWalkSpeed = details.hitMaxWalkSpeed;
	adjustSpeed = details.adjustSpeed;
	adjustMinDistance = details.adjustMinDistance;
	adjustMaxDistance = details.adjustMaxDistance;
	adjustAgle = details.adjustAgle;

	currentTarget = nullptr;


	inCombatTimer = 0;

	if (isPlayer)
	{
		listCounter = 0;
		moveDir = -1;
		swordGunAttack = false;
		//sword = GameObject.FindGameObjectWithTag("Sword");
		//sword.SetActive(false);
		//gun.SetActive(false);

		companion = static_cast<Companion*>(_companion);
		//player = static_cast<Player*>(owner);
		//character = static_cast<Character*>(owner);
	}
	else
	{
		//enemy = static_cast<Enemy*>(owner);
		//character = enemy;
	}

	character = static_cast<Character*>(owner);
	//assert(player != nullptr || enemy != nullptr);

	// Populate allMoves
	//allMoves

	// Setup the Combat Moves
	SetMoves();
	
}

CombatManager::~CombatManager()
{
}


// Update is called once per frame
void CombatManager::Update(float dT)
{
	//*******************************************************************
	if (isPlayer)
	{
		listCounter++;
		if (listCounter > 240)
		{
			/*
			enemyList.Clear();
			enemyArray = GameObject.FindGameObjectsWithTag("Enemy");
			for (int i = 0; i < enemyArray.Length; i++)
			{
				enemyList.Add(enemyArray[i]);
			}
			listCounter = 0;
			*/
		}


		if (attackDuration > -0.5f)
		{
			attackDuration -= dT;
		}
		if (attackDuration <= -0.5f && swordGunAttack)
		{
			//swordGunAttack = false;
			//sword.SetActive(false);
			//gun.SetActive(false);
			//companion.SetActive(true);
			//companion.transform.position = compPos;
		}

		float dist = FLT_MAX;
		/*
		foreach(GameObject obj in enemyList)
		{
			if (obj != null)
			{
				float distTo = (transform.position - obj.transform.position).magnitude;
				if (distTo < dist)
				{
					dist = distTo;
					currentTarget = obj.GetComponentInChildren<Character>();
				}
			}
		}*/

		if (isAimming && companion->ActiveSelf())
		{
			compPos = companion->GetPosition();
			companion->SetActive(false);
		}
		else if (isAimming == false && companion->ActiveSelf() == false 
			&& swordGunAttack == false)
		{
			companion->SetActive(true);
		}


	}
	//*******************************************************************
}


void  CombatManager::SetEnemyList(std::vector<Character*>* _enemyList)
{
	enemyList = _enemyList;
}


// Update the character's state
bool CombatManager::UpdateState(float stateTimer, float dT)
{
	int animationParameter = 0;

	if (inCombatTimer > 0)
	{
		inCombatTimer -= dT;
		if (inCombatTimer <= 0)
		{
			inCombat = false;
		}
	}

	if (comboTimer >= 0 && !(isAttacking || isAdjusting))
	{
		comboTimer += dT;
		if (comboTimer > maxComboTime)
		{
			comboTimer = -1;
			currentCombat = kNone;
		}
	}

	//is in combat
	if (isHit)
	{
		if (stateTimer < hitTime)
		{
			character->CurrentState(kHit);
			animationParameter = hitAnimationInfo;
			int power = hitAnimationInfo % 10;
			if ((HitPower)power == kPowerful)
			{
				int hitDirection = (hitAnimationInfo / 10) % 10;
				if (hitDirection >= 2)
				{
					character->ForceMove(hitMaxWalkSpeed * 0.5f, dT, hitDirection);
				}
				else
				{
					character->ForceMove(hitMaxWalkSpeed, dT, hitDirection);
				}
			}
			else if ((HitPower)power == kKO)
			{

			}
			if (resetHit)
			{
				character->LastState(kNoState);
				resetHit = false;
			}
			character->AnimationParameter(animationParameter);
		}
		else
		{
			isHit = false;
			stateTimer = -1;

			isAttacking = false;
			character->HasEffect(false);
			comboTimer = 0;
		}

		return true;
	}

	return false;
}

// Hit a character with an attack
void CombatManager::Hit(HitPosition pos, HitDirection dir, HitPower power, float _hitTime, float dmg)
{
	isHit = true;
	character->StateTimer(0);
	inCombat = true;
	inCombatTimer = inCombatDuration;
	hitAnimationInfo = (int)pos * 100 + (int)dir * 10 + (int)power;
	hitTime = _hitTime;
	resetHit = true;

	// Does damage based on what attack
	character->TakeDamage(dmg);

	isAttacking = false;
	isRolling = false;
	isAdjusting = false;
	isJumping = false;
	isMoving = false;

}

// Basic Attack Combos
void CombatManager::BasicCombo()
{
	//if can attack
	//if with in combat timer
	//NextCombat()
	if (!GetCanAttack())
	{
		return;
	}
	NextCombat();
	if (CheckTarget())
	{
		Attack();
	}
	else
	{
		Adjust();
	}
}

void CombatManager::SpecialCombat()
{
	if (!GetCanAttack())
	{
		return;
	}
	NextSpecial();
	if (CheckTarget())
	{
		Attack();
	}
	else
	{
		Adjust();
	}

}

// Gun Shooting
void CombatManager::GunShot()
{
	
}

// Sword Attack Combos
bool CombatManager::SwordCombo()
{
	//if can attack
	//if with in combat timer
	//NextCombat()
	if (!GetCanAttack())
	{
		return false;
	}
	NextSwordCombat();
	if (CheckTarget())
	{
		Attack();
		character->UseSpecial(25, true, false);
		return true;
	}
	else
	{
		Adjust();
	}

	return false;
}

bool CombatManager::SwordSpecialCombat()
{
	if (!GetCanAttack())
	{
		return false;
	}
	NextSwordSpecial();
	if (CheckTarget())
	{
		Attack();
		character->UseSpecial(50, true, false);
		return true;
	}
	else
	{
		Adjust();
	}
	return false;
}

// Melee Attacks
void CombatManager::Attack()
{
	inCombat = true;
	inCombatTimer = inCombatDuration;
	isAttacking = true;
	character->StateTimer(0);
	resetAttack = true;
}

// Ranged Attack
void CombatManager::Shoot()
{
	inCombat = true;
	inCombatTimer = inCombatDuration;
	isAttacking = true;
	character->StateTimer(0);
	resetAttack = true;

	// Update UI
	character->UseSpecial(33, false, true);

}

void CombatManager::Effect()
{
	// TODO change to a char[] comp later, maybe?
	if (currentTarget != nullptr &&  currentTarget->GetTag() != "Ghost")
	{
		float distance = math::distance(character->GetPosition(), currentTarget->GetPosition());
		HitDirection dir = kHitForward;

		if (distance <= currentEffectDistance)
		{
			//float angleFB = Vector3.Angle(currentTarget.charBody.transform.position - character->charBody.transform.position, currentTarget.charBody.transform.forward);
			//float angleLR = Vector3.Angle(currentTarget.charBody.transform.position - character->charBody.transform.position, currentTarget.charBody.transform.right);

			float angleFB = math::angleBetween(currentTarget->GetPosition() - character->GetPosition(), currentTarget->GetForward());
			float angleLR = math::angleBetween(currentTarget->GetPosition() - character->GetPosition(), currentTarget->GetRight());
			if (angleFB <= 45)
			{
				dir = kHitBackward;
			}
			else if (angleFB >= 135)
			{
				dir = kHitForward;
			}
			else
			{
				if (angleLR <= 45)
				{
					dir = kHitLeft;
				}
				else if (angleLR >= 135)
				{
					dir = kHitRight;
				}
			}
			if (currentDirection == kDirRight)
			{
				if (dir == kHitForward)
				{
					dir = kHitLeft;
				}
				else if (dir == kHitBackward)
				{
					dir = kHitRight;
				}
				else if (dir == kHitLeft)
				{
					dir = kHitBackward;
				}
				else if (dir == kHitRight)
				{
					dir = kHitForward;
				}
			}
			else if (currentDirection == kDirLeft)
			{
				if (dir == kHitForward)
				{
					dir = kHitRight;
				}
				else if (dir == kHitBackward)
				{
					dir = kHitLeft;
				}
				else if (dir == kHitLeft)
				{
					dir = kHitForward;
				}
				else if (dir == kHitRight)
				{
					dir = kHitBackward;
				}
			}

			// If attack is a sword attack, hit multiple enemies
			if (currentCombat == kSwordAttackR || currentCombat == kSwordAttackRL ||
				currentCombat == kSwordAttackComboLL)
			{
				for (uint32_t i = 0; i < enemyList->size(); i++)
				{
					if (math::distance(enemyList->at(i)->GetPosition() , character->GetPosition()) <
						allMoves[(int)currentCombat - 1].ED)
					{
						enemyList->at(i)->GetCombatManager()->Hit(enemyList->at(i)->GetCombatManager()->GetCurrentHitPos(), dir, currentPower, currentHitTime, currentDmgAmount);
					}
				}
			}
			else
			{
				currentTarget->GetCombatManager()->Hit(currentTarget->GetCombatManager()->GetCurrentHitPos(), dir, currentPower, currentHitTime, currentDmgAmount);
			}
		}

	}
}

// Check the target
bool CombatManager::CheckTarget()
{
	if (currentTarget == nullptr)
	{
		return true;
	}
	else
	{
		float distance = math::distance(character->GetPosition(), currentTarget->GetPosition());
		float angle = math::angleBetween(currentTarget->GetPosition() - character->GetPosition(), character->GetForward());

		if (distance < adjustMinDistance && angle < adjustAgle)
		{
			return true;
		}
		else if (distance > adjustMaxDistance)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

}

// Check to see if the player has a valid ranged target
bool CombatManager::CheckRangeTarget()
{
	if (aimTarget == nullptr)
	{
		return false;
	}
	else
	{
		float distance = math::distance(character->GetPosition(), aimTarget->GetPosition());
		//float angle = Vector3.Angle(currentTarget.charBody.transform.position - character->charBody.transform.position, character->charBody.transform.forward);

		if (distance < minShotDistance)
		{
			return false;
		}
		else if (distance > maxShotDistance)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

void CombatManager::Adjust()
{
	isAdjusting = true;
}

void CombatManager::Dodge(int dir)
{
	if (!GetCanDodge())
	{
		return;
	}
	dodgeDirection = dir;
	character->StateTimer(0);
	isDodging = true;
}

void CombatManager::Roll()
{
	if (!GetCanRoll())
	{
		return;
	}
	character->StateTimer(0);
	isRolling = true;
}

void CombatManager::AimMove(int dir)
{
	if (!GetCanMove())
	{
		return;
	}
	moveDir = dir;
}

// Function to perform the combat that was passed.
//void CombatManager::PerformAction(Combat _input)
//{
//	currentCombat = _input;
//	// TODO: A Variable for the Sound in the CombatMoveDetails Struct.
//	//combatAudio.PlayOneShot(punchFX);
//
//	// Remove 1 from the current combat because of the None in the Enum.
//	CombatMoveDetails currentMoveDetails = allMoves[(int)currentCombat - 1];
//	currentAttackTime = currentMoveDetails.AT;
//	currentEffectTime = currentMoveDetails.ET;
//	currentEffectDistance = currentMoveDetails.ED;
//	currentDmgAmount = currentMoveDetails.Dmg;
//	currentDirection = currentMoveDetails.dir;
//	currentPower = currentMoveDetails.power;
//	currentHitPos = currentMoveDetails.pos;
//	currentHitTime = currentMoveDetails.HT;
//
//	Attack();
//
//}

// Function to perform the combat that was passed.
void CombatManager::PerformAction(Action _action)
{

	Combat _input = _action.combat;

	if (kNone == _input)
	{
		// Perform the Action even though the Combat is set to None.
		if (_action.name == "Face_Towards_The_Enemy")
		{
			isTurning = true;
			return;
		}
		else
		{
			return;
		}
		
		/*switch (_action.name)
		{
		case "Face_Towards_The_Enemy":
			isTurning = true;
			return;
		default:
			return;
		}*/

	}

	currentCombat = _input;

	// Remove 1 from the current combat because of the None in the Enum.
	CombatMoveDetails currentMoveDetails = allMoves[(int)currentCombat - 1];
	currentAttackTime = currentMoveDetails.AT;
	currentEffectTime = currentMoveDetails.ET;
	currentEffectDistance = currentMoveDetails.ED;
	currentDmgAmount = currentMoveDetails.Dmg;
	currentDirection = currentMoveDetails.dir;
	currentPower = currentMoveDetails.power;
	currentHitPos = currentMoveDetails.pos;
	currentHitTime = currentMoveDetails.HT;
	currentAttackDistance = currentMoveDetails.strikeDistance;
	//currentSFX = allMoves[(int)currentCombat - 1].CombatSFX;

	// Move according to the Attack Distance.

	// Variable for the Sound

	Attack();

}

void CombatManager::NextCombat()
{
	if (currentCombat == kNone)
	{
		//combatAudio.PlayOneShot(punchFX);
		currentCombat = kPunchJabL;
	}
	else if (currentCombat == kPunchJabL)
	{
		//combatAudio.PlayOneShot(punchFX);
		currentCombat = kPunchJabR;
	}
	else if (currentCombat == kPunchJabR)
	{
		//combatAudio.PlayOneShot(punchFX);
		currentCombat = kPunchHookL;
	}
	else if (currentCombat == kPunchHookL)
	{
		//combatAudio.PlayOneShot(punchFX);
		currentCombat = kPunchHookR;
	}
	else if (currentCombat == kPunchHookR)
	{
		//combatAudio.PlayOneShot(punchFX);
		currentCombat = kKickStraightMidR;
	}
	else if (currentCombat == kKickStraightMidR)
	{
		//combatAudio.PlayOneShot(kickFX);
		currentCombat = kPunchJabL;
	}
	else
	{
		//combatAudio.PlayOneShot(punchFX);
		currentCombat = kPunchJabL;
	}

	// Because of None in the Enum, subtract by 1.
	CombatMoveDetails currentMoveDetails = allMoves[(int)currentCombat - 1];
	currentAttackTime = currentMoveDetails.AT;
	currentEffectTime = currentMoveDetails.ET;
	currentEffectDistance = currentMoveDetails.ED;
	currentDmgAmount = currentMoveDetails.Dmg;
	currentDirection = currentMoveDetails.dir;
	currentPower = currentMoveDetails.power;
	currentHitPos = currentMoveDetails.pos;
	currentHitTime = currentMoveDetails.HT;

}

void CombatManager::NextSpecial()
{
	if (currentCombat == kNone)
	{
		//combatAudio.PlayOneShot(kickFX);
		currentCombat = kKickAxeKick;
	}
	else if (currentCombat == kPunchJabL)
	{
		//combatAudio.PlayOneShot(kickFX);
		currentCombat = kKickAxeKick;
	}
	else if (currentCombat == kPunchJabR)
	{
		//combatAudio.PlayOneShot(kickFX);
		currentCombat = kKickAxeKick;
	}
	else if (currentCombat == kPunchHookL)
	{
		//combatAudio.PlayOneShot(kickFX);
		currentCombat = kKickAxeKick;
	}
	else if (currentCombat == kPunchHookR)
	{
		//combatAudio.PlayOneShot(kickFX);
		currentCombat = kKickAxeKick;
	}
	else if (currentCombat == kKickStraightMidR)
	{
		//combatAudio.PlayOneShot(kickFX);
		currentCombat = kKickHorseKick;
	}
	else
	{
		//combatAudio.PlayOneShot(kickFX);
		currentCombat = kKickAxeKick;
	}

	// Again reduce by 1 because of enum definition.
	CombatMoveDetails currentMoveDetails = allMoves[(int)currentCombat - 1];

	currentAttackTime = currentMoveDetails.AT;
	currentEffectTime = currentMoveDetails.ET;
	currentEffectDistance = currentMoveDetails.ED;
	currentDmgAmount = currentMoveDetails.Dmg;
	currentDirection = currentMoveDetails.dir;
	currentPower = currentMoveDetails.power;
	currentHitPos = currentMoveDetails.pos;
	currentHitTime = currentMoveDetails.HT;
}

// --------------------------------------------------------------------------------------------
// Sword Attack
void CombatManager::NextSwordCombat()
{
	if (currentCombat == kNone)
	{
		//combatAudio.PlayOneShot(swordFX);
		currentCombat = kSwordAttackR;
	}
	else if (currentCombat == kSwordAttackR)
	{
		//combatAudio.PlayOneShot(swordFX);
		currentCombat = kSwordAttackRL;
	}
	else
	{
		//combatAudio.PlayOneShot(swordFX);
		currentCombat = kSwordAttackR;
	}

	// Reduce by 1 because of enum.
	CombatMoveDetails currentMoveDetails = allMoves[(int)currentCombat - 1];

	currentAttackTime = currentMoveDetails.AT;
	currentEffectTime = currentMoveDetails.ET;
	currentEffectDistance = currentMoveDetails.ED;
	currentDmgAmount = currentMoveDetails.Dmg;
	currentDirection = currentMoveDetails.dir;
	currentPower = currentMoveDetails.power;
	currentHitPos = currentMoveDetails.pos;
	currentHitTime = currentMoveDetails.HT;

	// TODO
	//sword.SetActive(true);
	compPos = companion->GetPosition();
	//companion.SetActive(false);
	swordGunAttack = true;
	attackDuration = currentAttackTime;
}

void CombatManager::NextSwordSpecial()
{
	if (currentCombat == kNone)
	{
		//combatAudio.PlayOneShot(swordFX);
		currentCombat = kSwordAttackComboLL;
	}
	else if (currentCombat == kPunchJabL)
	{
		//combatAudio.PlayOneShot(swordFX);
		currentCombat = kSwordAttackSpU;
	}
	else if (currentCombat == kPunchJabR)
	{
		//combatAudio.PlayOneShot(swordFX);
		currentCombat = kSwordAttackSpU;
	}
	else if (currentCombat == kPunchHookL)
	{
		//combatAudio.PlayOneShot(swordFX);
		currentCombat = kSwordAttackSpU;
	}
	else if (currentCombat == kPunchHookR)
	{
		//combatAudio.PlayOneShot(swordFX);
		currentCombat = kSwordAttackSpU;
	}
	else if (currentCombat == kKickStraightMidR)
	{
		//combatAudio.PlayOneShot(swordFX);
		currentCombat = kSwordAttackSpU;
	}
	else if (currentCombat == kSwordAttackRL)
	{
		//combatAudio.PlayOneShot(swordFX);
		currentCombat = kSwordAttackSpU;
	}
	else
	{
		//combatAudio.PlayOneShot(swordFX);
		currentCombat = kSwordAttackComboLL;
	}

	// 
	CombatMoveDetails currentMoveDetails = allMoves[(int)currentCombat - 1];

	currentAttackTime = currentMoveDetails.AT;
	currentEffectTime = currentMoveDetails.ET;
	currentEffectDistance = currentMoveDetails.ED;
	currentDmgAmount = currentMoveDetails.Dmg;
	currentDirection = currentMoveDetails.dir;
	currentPower = currentMoveDetails.power;
	currentHitPos = currentMoveDetails.pos;
	currentHitTime = currentMoveDetails.HT;

	// TODO
	//sword.SetActive(true);
	compPos = companion->GetPosition();
	//companion.SetActive(false);
	swordGunAttack = true;
	attackDuration = currentAttackTime;
}
// --------------------------------------------------------------------------------------------

void CombatManager::SetMoves()
{
	// Punch_Jab_L
	defaultMove.AT = 0.7f;
	defaultMove.AD = 0.0f;
	defaultMove.ET = 0.2f;
	defaultMove.ED = 1.0f;
	defaultMove.HT = 0.2f;
	defaultMove.Dmg = 3.f;
	defaultMove.pos = kHigh;
	defaultMove.dir = kDirForward;
	defaultMove.power = kWeak;
	allMoves.push_back(defaultMove);

	// Punch_Jab_R
	defaultMove.AT = 0.5f;
	defaultMove.ET = 0.2f;
	defaultMove.ED = 1.0f;
	defaultMove.HT = 1.0f;
	defaultMove.Dmg = 3.f;
	defaultMove.AD = 0.0f;
	defaultMove.pos = kHigh;
	defaultMove.dir = kDirForward;
	defaultMove.power = kWeak;
	allMoves.push_back(defaultMove);

	// Punch_Hook_L
	defaultMove.AT = 0.65f;
	defaultMove.ET = 0.2f;
	defaultMove.ED = 1.0f;
	defaultMove.HT = 0.2f;
	defaultMove.Dmg = 5.0f;
	defaultMove.AD = 0.0f;
	defaultMove.pos = kHigh;
	defaultMove.dir = kDirForward;
	defaultMove.power = kWeak;
	allMoves.push_back(defaultMove);

	// Punch_Hook_R
	defaultMove.AT = 0.65f;
	defaultMove.ET = 0.2f;
	defaultMove.ED = 1.0f;
	defaultMove.HT = 0.5f;
	defaultMove.Dmg = 5.0f;
	defaultMove.AD = 0.0f;
	defaultMove.pos = kHigh;
	defaultMove.dir = kDirForward;
	defaultMove.power = kWeak;
	allMoves.push_back(defaultMove);

	// Punch_Upper_Cut_L
	defaultMove.AT = 0.8f;
	defaultMove.ET = 0.5f;
	defaultMove.ED = 1.0f;
	defaultMove.HT = 1.0f;
	defaultMove.Dmg = 7.0f;
	defaultMove.AD = 0.0f;
	defaultMove.pos = kHigh;
	defaultMove.dir = kDirForward;
	defaultMove.power = kWeak;
	allMoves.push_back(defaultMove);
	
	// Punch_Upper_Cut_R
	defaultMove.AT = 0.8f;
	defaultMove.ET = 0.5f;
	defaultMove.ED = 1.0f;
	defaultMove.HT = 1.0f;
	defaultMove.Dmg = 7.0f;
	defaultMove.AD = 0.0f;
	defaultMove.pos = kHigh;
	defaultMove.dir = kDirForward;
	defaultMove.power = kWeak;
	allMoves.push_back(defaultMove);

	// Kick_Straight_Mid_R
	defaultMove.AT = 1.0f;
	defaultMove.ET = 0.6f;
	defaultMove.ED = 1.0f;
	defaultMove.HT = 0.8f;
	defaultMove.Dmg = 10.0f;
	defaultMove.AD = 0.0f;
	defaultMove.pos = kHigh;
	defaultMove.dir = kDirForward;
	defaultMove.power = kWeak;
	allMoves.push_back(defaultMove);

	// Kick_Knee
	defaultMove.AT = 1.0f;
	defaultMove.ET = 0.6f;
	defaultMove.ED = 1.0f;
	defaultMove.HT = 0.8f;
	defaultMove.Dmg = 10.0f;
	defaultMove.AD = 0.0f;
	defaultMove.pos = kMid;
	defaultMove.dir = kDirForward;
	defaultMove.power = kWeak;
	allMoves.push_back(defaultMove);

	// Kick_Axe_Kick
	defaultMove.AT = 1.2f;
	defaultMove.ET = 0.5f;
	defaultMove.ED = 1.0f;
	defaultMove.HT = 0.8f;
	defaultMove.Dmg = 10.0f;
	defaultMove.AD = 0.0f;
	defaultMove.pos = kHigh;
	defaultMove.dir = kDirForward;
	defaultMove.power = kWeak;
	allMoves.push_back(defaultMove);

	// Kick_Horse_Kick
	defaultMove.AT = 1.0f;
	defaultMove.ET = 0.4f;
	defaultMove.ED = 1.0f;
	defaultMove.HT = 1.0f;
	defaultMove.Dmg = 3.0f;
	defaultMove.AD = 0.0f;
	defaultMove.pos = kHigh;
	defaultMove.dir = kDirForward;
	defaultMove.power = kWeak;
	allMoves.push_back(defaultMove);

	// Sword_Attack_R
	defaultMove.name = "SwordR";
	defaultMove.AT = 2.02f;
	defaultMove.ET = 1.0f;
	defaultMove.ED = 1.25f;
	defaultMove.HT = 1.0f;
	defaultMove.Dmg = 10.0f;
	defaultMove.AD = 0.0f;
	defaultMove.pos = kMid;
	defaultMove.dir = kDirLeft;
	defaultMove.power = kWeak;
	allMoves.push_back(defaultMove);

	// Sword_Attack_RL
	defaultMove.name = "SwordRL";
	defaultMove.AT = 1.8f;
	defaultMove.ET = 0.5f;
	defaultMove.ED = 1.25f;
	defaultMove.HT = 1.0f;
	defaultMove.Dmg = 10.0f;
	defaultMove.AD = 0.0f;
	defaultMove.pos = kHigh;
	defaultMove.dir = kDirRight;
	defaultMove.power = kWeak;
	allMoves.push_back(defaultMove);

	// Sword_Attack_Sp_U
	defaultMove.name = "SwordSp";
	defaultMove.AT = 2.5f;
	defaultMove.ET = 0.5f;
	defaultMove.ED = 1.25f;
	defaultMove.HT = 1.0f;
	defaultMove.Dmg = 15.0f;
	defaultMove.AD = 0.0f;
	defaultMove.pos = kHigh;
	defaultMove.dir = kDirForward;
	defaultMove.power = kPowerful;
	allMoves.push_back(defaultMove);

	// Sword_Attack_Combo_LL
	defaultMove.name = "SwordCombo";
	defaultMove.AT = 2.0f;
	defaultMove.ET = 0.5f;
	defaultMove.ED = 1.25f;
	defaultMove.HT = 0.8f;
	defaultMove.Dmg = 20.0f;
	defaultMove.AD = 0.0f;
	defaultMove.pos = kHigh;
	defaultMove.dir = kDirRight;
	defaultMove.power = kPowerful;
	allMoves.push_back(defaultMove);
}

//-------------------------------------------------------------------------------------------------
// Setters

void CombatManager::SetAdjustMinDistance(float minDistance)
{
	adjustMinDistance = minDistance;
}


void CombatManager::SetAdjustMaxDistance(float maxDistance)
{
	adjustMaxDistance = maxDistance;
}

// Set the Move Direction
void CombatManager::SetMoveDir(int _moveDir)
{
	moveDir = _moveDir;
}

// Set the current target
void CombatManager::SetCurrentTarget(Character* _currentTarget)
{
	currentTarget = _currentTarget;
}

// Set the current aim target
void CombatManager::SetAimTarget(Character* _aimTarget)
{
	aimTarget = _aimTarget;
}

// Set the current hit position
void CombatManager::SetCurrentHitPos(HitPosition _currentHitPos)
{
	currentHitPos = _currentHitPos;
}

// Set if is aiming
void CombatManager::SetIsAimming(bool _isAimming)
{
	isAimming = _isAimming;
}

// Set if is moving
void CombatManager::SetIsMoving(bool _isMoving)
{
	isMoving = _isMoving;
}

// Set if is sprinting
void CombatManager::SetIsSprinting(bool _isSprinting)
{
	isSprinting = _isSprinting;
}

// Set if is turning
void CombatManager::SetIsTurning(bool _isTurning)
{
	isTurning = _isTurning;
}

// Set if is jumping
void CombatManager::SetIsJumping(bool _isJumping)
{
	isJumping = _isJumping;
}

// Set if is attacking
void CombatManager::SetIsAttacking(bool _isAttacking)
{
	isAttacking = _isAttacking;
}

// Reset attack
void CombatManager::SetResetAttack(bool _resetAttack)
{
	resetAttack = _resetAttack;
}

// Set if is rolling
void CombatManager::SetIsRolling(bool _isRolling)
{
	isRolling = _isRolling;
}

// Set if is dodging
void CombatManager::SetIsDodging(bool _isDodging)
{
	isDodging = _isDodging;
}

// Set the hit time
void CombatManager::SetHitTime(float _hitTime)
{
	hitTime = _hitTime;
}

// Set if is adjusting
void CombatManager::SetIsAdjusting(bool _isAdjusting)
{
	isAdjusting = _isAdjusting;
}

// Set the combo timer
void CombatManager::SetComboTimer(float _comboTimer)
{
	comboTimer = _comboTimer;
}

//-------------------------------------------------------------------------------------------------
// Getters

// Return if the character can move
bool CombatManager::GetCanMove()
{
	return !(isRolling || isHit || isAttacking || isDodging || isAdjusting);
}

// Return if the character can jump
bool CombatManager::GetCanJump()
{
	return !(isRolling || isJumping || isAimming || isAttacking || isDodging || isAdjusting);
}

// Return if the character can attack
bool CombatManager::GetCanAttack()
{
	return !(isRolling || isJumping || isAttacking || isDodging || isAdjusting);
}

// Return if can set the next attack
bool CombatManager::GetCanSetNextAttack()
{
	return isAttacking;
}

// Return if the character can dodge
bool CombatManager::GetCanDodge()
{
	return !(isRolling || isJumping || isAttacking || isDodging || isAdjusting);
}

// Return if the character can roll
bool CombatManager::GetCanRoll()
{
	return !(isRolling || isJumping || isAimming || isAttacking || isDodging || isAdjusting);
}

// Return the move direction
int CombatManager::GetMoveDir()
{
	return moveDir;
}

// Return the current target
Character* CombatManager::GetCurrentTarget()
{
	return currentTarget;
}

// Return the current aim target
Character* CombatManager::GetAimTarget()
{
	return aimTarget;
}

// Return if the character is in combat
bool CombatManager::GetInCombat()
{
	return inCombat;
}

// Return the current hit position
HitPosition CombatManager::GetCurrentHitPos()
{
	return currentHitPos;
}

// Return if the character is aiming
bool CombatManager::GetIsAimming()
{
	return isAimming;
}

// Return if the character is moving
bool CombatManager::GetIsMoving()
{
	return isMoving;
}

// Return if the character is sprinting
bool CombatManager::GetIsSprinting()
{
	return isSprinting;
}

// Return if the character is turning
bool CombatManager::GetIsTurning()
{
	return isTurning;
}

// Return if the character is jumping
bool CombatManager::GetIsJumping()
{
	return isJumping;
}

// Return the jump up time
float CombatManager::GetJumpUpTime()
{
	return jumpUpTime;
}

// Return the jump air time
float CombatManager::GetJumpAirTime()
{
	return jumpAirTime;
}

// Return the jump down time
float CombatManager::GetJumpDownTime()
{
	return jumpDownTime;
}

// Return if the character is attacking
bool CombatManager::GetIsAttacking()
{
	return isAttacking;
}

// Return the current attack time
float CombatManager::GetCurrentAttackTime()
{
	return currentAttackTime;
}

// Return the current effect time
float CombatManager::GetCurrentEffectTime()
{
	return currentEffectTime;
}

// Return if reset attack
bool CombatManager::GetResetAttack()
{
	return resetAttack;
}

// Return if the character is dodging
bool CombatManager::GetIsDodging()
{
	return isDodging;
}

// Return the dodge time
float CombatManager::GetDodgeTime()
{
	return dodgeTime;
}

// Return the dodge direction
int CombatManager::GetDodgeDirection()
{
	return dodgeDirection;
}

// Return if the character is rolling
bool CombatManager::GetIsRolling()
{
	return isRolling;
}

// Return the roll time
float CombatManager::GetRollTime()
{
	return rollTime;
}

// Return the roll speed
float CombatManager::GetRollSpeed()
{
	return rollSpeed;
}

// Return if the character is hit
bool CombatManager::GetIsHit()
{
	return isHit;
}

// Return the hit time
float CombatManager::GetHitTime()
{
	return hitTime;
}

// Return if the character is adjusting
bool CombatManager::GetIsAdjusting()
{
	return isAdjusting;
}

// Return the adjust speed
float CombatManager::GetAdjustSpeed()
{
	return adjustSpeed;
}

// Return the adjust minimum distance
float CombatManager::GetAdjustMinDistance()
{
	return adjustMinDistance;
}

// Return the adjust maximum distance
float CombatManager::GetAdjustMaxDistance()
{
	return adjustMaxDistance;
}

// Return the adjust angle
float CombatManager::GetAdjustAngle()
{
	return adjustAgle;
}

// Return the current combat
Combat CombatManager::GetCurrentCombat()
{
	return currentCombat;
}

// Return the combo timer
float CombatManager::GetComboTimer()
{
	return comboTimer;
}

float CombatManager::GetTimeBetweenAttacks()
{
	return timeBetweenAttacks;
}