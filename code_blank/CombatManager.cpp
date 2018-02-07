#include "CombatManager.h"

using namespace tofu;

CombatManager::CombatManager(bool _isPlayer, void* _companion, void* owner)
{
	player = nullptr;
	enemy = nullptr;

	isPlayer = _isPlayer;

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
		player = static_cast<Player*>(owner);

		// Make a list of enemies in the scene
		/*enemyList = new List<GameObject>();
		enemyArray = GameObject.FindGameObjectsWithTag("Enemy");
		for (int i = 0; i < enemyArray.Length; i++)
		{
			enemyList.Add(enemyArray[i]);
		}
		*/
	}
	else
	{
		enemy = static_cast<Enemy*>(owner);
	}

	assert(player != nullptr || enemy != nullptr);
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

		if (isAimming && companion->ActiveSelf)
		{
			compPos = companion->GetPosition();
			companion->SetActive(false);
		}
		else if (!isAimming && !companion->ActiveSelf && !swordGunAttack)
		{
			companion->SetActive(true);
		}


	}
	//*******************************************************************
}

/*
public void SetChar(Character _char)
{
	m_char = _char;
}*/


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
			player->CurrentState(kHit);
			animationParameter = hitAnimationInfo;
			int power = hitAnimationInfo % 10;
			if ((HitPower)power == kPowerful)
			{
				int hitDirection = (hitAnimationInfo / 10) % 10;
				if (hitDirection >= 2)
				{
					m_char.ForceMove(hitMaxWalkSpeed * 0.5f, hitDirection);
				}
				else
				{
					m_char.ForceMove(hitMaxWalkSpeed, hitDirection);
				}
			}
			else if ((HitPower)power == kKO)
			{

			}
			if (resetHit)
			{
				m_char.LastState = Character.CharacterState.none;
				resetHit = false;
			}
			m_char.AnimationParameter = animationParameter;
		}
		else
		{
			isHit = false;
			stateTimer = -1;

			isAttacking = false;
			m_char.HasEffect = false;
			comboTimer = 0;
		}

		return true;
	}

	return false;
}

// Hit a character with an attack
void CombatManager::Hit(HitPosition pos, HitDirection dir, HitPower power, float hitTime, float dmg, float delay = 0, bool dmgDelay = false)
{
	//
	isHit = true;
	m_char.StateTimer = 0;
	inCombat = true;
	inCombatTimer = inCombatDuration;
	hitAnimationInfo = (int)pos * 100 + (int)dir * 10 + (int)power;
	HitTime = hitTime;
	resetHit = true;

	// Does damage based on what attack
	if (!dmgDelay)
	{
		this.GetComponent<Humanoid>().TakeDamag(dmg);
	}
	else
	{
		StartCoroutine(DelayBeforeDamage(delay, dmg));
	}

	//Debug.Log(gameObject.name + ": Takes " + dmg + " damage");

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
	if (!canAttack)
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
	if (!canAttack)
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
	if (!canAttack || !isAimming || aimTarget == null)
	{
		return;
	}

	if (CheckRangeTarget())
	{
		swordGunAttack = true;
		//gun.SetActive(true);
		combatAudio.clip = gunShotFX;
		combatAudio.PlayDelayed(0.5f);
		currentCombat = Combat.Gun_Shoot;

		// Because of None in the Enum, subtract by 1.
		CombatMoveDetails currentMoveDetails = allMoves[(int)currentCombat - 1];
		//Debug.Log(currentCombat + " " + currentMoveDetails.name);
		currentAttackTime = currentMoveDetails.AT;
		currentEffectTime = currentMoveDetails.ET;
		currentEffetDistance = currentMoveDetails.ED;
		currentDmgAmount = currentMoveDetails.Dmg;
		currentDirection = currentMoveDetails.Dir;
		currentPower = currentMoveDetails.Power;
		currentHitPos = currentMoveDetails.Pos;
		currentHitTime = currentMoveDetails.HT;

		attackDuration = currentAttackTime;
		Shoot();

		// TODO Modify hit dir later
		aimTarget.m_combat.Hit(aimTarget.m_combat.currentHitPos, HitDirection.backward, currentPower, currentHitTime, currentDmgAmount, 0.5f, true);
	}
}

// Sword Attack Combos
void CombatManager::SwordCombo()
{
	//if can attack
	//if with in combat timer
	//NextCombat()
	if (!canAttack)
	{
		return;
	}
	NextSwordCombat();
	if (CheckTarget())
	{
		Attack();
		m_char.GetComponent<PC>().UseSpecial(25, true);
	}
	else
	{
		Adjust();
	}
}

void CombatManager::SwordSpecialCombat()
{
	if (!canAttack)
	{
		return;
	}
	NextSwordSpecial();
	if (CheckTarget())
	{
		Attack();
		m_char.GetComponent<PC>().UseSpecial(50, true);
	}
	else
	{
		Adjust();
	}

}

// Melee Attacks
void CombatManager::Attack()
{
	inCombat = true;
	inCombatTimer = inCombatDuration;
	isAttacking = true;
	m_char.StateTimer = 0;
	resetAttack = true;
}

// Ranged Attack
void CombatManager::Shoot()
{
	inCombat = true;
	inCombatTimer = inCombatDuration;
	isAttacking = true;
	m_char.StateTimer = 0;
	resetAttack = true;

	// Update UI
	m_char.GetComponent<PC>().Shoot();
}

void CombatManager::Effect()
{

	if (currentTarget != null && currentTarget.tag != "Ghost")
	{

		float distance = Vector3.Distance(m_char.charBody.transform.position, currentTarget.charBody.transform.position);
		HitDirection dir = HitDirection.forward;

		if (distance <= currentEffetDistance)
		{
			float angleFB = Vector3.Angle(currentTarget.charBody.transform.position - m_char.charBody.transform.position, currentTarget.charBody.transform.forward);
			float angleLR = Vector3.Angle(currentTarget.charBody.transform.position - m_char.charBody.transform.position, currentTarget.charBody.transform.right);
			if (angleFB <= 45)
			{
				dir = HitDirection.backward;
			}
			else if (angleFB >= 135)
			{
				dir = HitDirection.forward;
			}
			else
			{
				if (angleLR <= 45)
				{
					dir = HitDirection.left;
				}
				else if (angleLR >= 135)
				{
					dir = HitDirection.right;
				}
			}
			if (currentDirection == CombatDirection.right)
			{
				if (dir == HitDirection.forward)
				{
					dir = HitDirection.left;
				}
				else if (dir == HitDirection.backward)
				{
					dir = HitDirection.right;
				}
				else if (dir == HitDirection.left)
				{
					dir = HitDirection.backward;
				}
				else if (dir == HitDirection.right)
				{
					dir = HitDirection.forward;
				}
			}
			else if (currentDirection == CombatDirection.left)
			{
				if (dir == HitDirection.forward)
				{
					dir = HitDirection.right;
				}
				else if (dir == HitDirection.backward)
				{
					dir = HitDirection.left;
				}
				else if (dir == HitDirection.left)
				{
					dir = HitDirection.forward;
				}
				else if (dir == HitDirection.right)
				{
					dir = HitDirection.backward;
				}
			}

			// If attack is a sword attack, hit multiple enemies
			if (currentCombat == Combat.Sword_Attack_R || currentCombat == Combat.Sword_Attack_RL ||
				currentCombat == Combat.Sword_Attack_Combo_LL)
			{
				foreach(GameObject obj in enemyList)
				{
					if (Vector3.Distance(obj.transform.position, gameObject.transform.position) <
						allMoves[(int)currentCombat - 1].ED)
					{
						obj.GetComponent<CombatManager>().Hit(currentTarget.m_combat.currentHitPos, dir, currentPower, currentHitTime, currentDmgAmount, 0.5f, true);
					}
				}
			}
			else
			{
				currentTarget.m_combat.Hit(currentTarget.m_combat.currentHitPos, dir, currentPower, currentHitTime, currentDmgAmount, 0.5f, true);
			}
		}

	}
}

bool CombatManager::CheckTarget()
{
	if (currentTarget == null)
	{
		return true;
	}
	else
	{
		float distance = Vector3.Distance(m_char.charBody.transform.position, currentTarget.charBody.transform.position);
		float angle = Vector3.Angle(currentTarget.charBody.transform.position - m_char.charBody.transform.position, m_char.charBody.transform.forward);

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
	if (aimTarget == null)
	{
		return false;
	}
	else
	{
		float distance = Vector3.Distance(m_char.charBody.transform.position, aimTarget.charBody.transform.position);
		//float angle = Vector3.Angle(currentTarget.charBody.transform.position - m_char.charBody.transform.position, m_char.charBody.transform.forward);

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

void Dodge(int dir)
{
	if (!canDodge)
	{
		return;
	}
	dodgeDirection = dir;
	m_char.StateTimer = 0;
	isDodging = true;
}

void CombatManager::Roll()
{
	if (!canRoll)
	{
		return;
	}
	m_char.StateTimer = 0;
	isRolling = true;
}

void CombatManager::AimMove(int dir)
{
	if (!canMove)
	{
		return;
	}
	moveDir = dir;
}

// Function to perform the combat that was passed.
void CombatManager::PerformAction(Combat _input)
{
	currentCombat = _input;
	// TODO: A Variable for the Sound in the CombatMoveDetails Struct.
	combatAudio.PlayOneShot(punchFX);

	// Remove 1 from the current combat because of the None in the Enum.
	CombatMoveDetails currentMoveDetails = allMoves[(int)currentCombat - 1];
	Debug.Log(currentCombat + " " + currentMoveDetails.name);
	currentAttackTime = currentMoveDetails.AT;
	currentEffectTime = currentMoveDetails.ET;
	currentEffetDistance = currentMoveDetails.ED;
	currentDmgAmount = currentMoveDetails.Dmg;
	currentDirection = currentMoveDetails.Dir;
	currentPower = currentMoveDetails.Power;
	currentHitPos = currentMoveDetails.Pos;
	currentHitTime = currentMoveDetails.HT;

	Attack();

}

void CombatManager::NextCombat()
{
	if (currentCombat == Combat.none)
	{
		combatAudio.PlayOneShot(punchFX);
		currentCombat = Combat.punch_Jab_L;
	}
	else if (currentCombat == Combat.punch_Jab_L)
	{
		combatAudio.PlayOneShot(punchFX);
		currentCombat = Combat.punch_Jab_R;
	}
	else if (currentCombat == Combat.punch_Jab_R)
	{
		combatAudio.PlayOneShot(punchFX);
		currentCombat = Combat.punch_Hook_L;
	}
	else if (currentCombat == Combat.punch_Hook_L)
	{
		combatAudio.PlayOneShot(punchFX);
		currentCombat = Combat.punch_Hook_R;
	}
	else if (currentCombat == Combat.punch_Hook_R)
	{
		combatAudio.PlayOneShot(punchFX);
		currentCombat = Combat.kick_Straight_Mid_R;
	}
	else if (currentCombat == Combat.kick_Straight_Mid_R)
	{
		combatAudio.PlayOneShot(kickFX);
		currentCombat = Combat.punch_Jab_L;
	}
	else
	{
		combatAudio.PlayOneShot(punchFX);
		currentCombat = Combat.punch_Jab_L;
	}

	// Because of None in the Enum, subtract by 1.
	CombatMoveDetails currentMoveDetails = allMoves[(int)currentCombat - 1];
	//Debug.Log(currentCombat + " " + currentMoveDetails.name);
	currentAttackTime = currentMoveDetails.AT;
	currentEffectTime = currentMoveDetails.ET;
	currentEffetDistance = currentMoveDetails.ED;
	currentDmgAmount = currentMoveDetails.Dmg;
	currentDirection = currentMoveDetails.Dir;
	currentPower = currentMoveDetails.Power;
	currentHitPos = currentMoveDetails.Pos;
	currentHitTime = currentMoveDetails.HT;

}

void CombatManager::NextSpecial()
{
	if (currentCombat == Combat.none)
	{
		combatAudio.PlayOneShot(kickFX);
		currentCombat = Combat.kick_AxeKick;
	}
	else if (currentCombat == Combat.punch_Jab_L)
	{
		combatAudio.PlayOneShot(kickFX);
		currentCombat = Combat.kick_AxeKick;
	}
	else if (currentCombat == Combat.punch_Jab_R)
	{
		combatAudio.PlayOneShot(kickFX);
		currentCombat = Combat.kick_AxeKick;
	}
	else if (currentCombat == Combat.punch_Hook_L)
	{
		combatAudio.PlayOneShot(kickFX);
		currentCombat = Combat.kick_AxeKick;
	}
	else if (currentCombat == Combat.punch_Hook_R)
	{
		combatAudio.PlayOneShot(kickFX);
		currentCombat = Combat.kick_AxeKick;
	}
	else if (currentCombat == Combat.kick_Straight_Mid_R)
	{
		combatAudio.PlayOneShot(kickFX);
		currentCombat = Combat.kick_HorseKick;
	}
	else
	{
		combatAudio.PlayOneShot(kickFX);
		currentCombat = Combat.kick_AxeKick;
	}

	// Again reduce by 1 because of enum definition.
	CombatMoveDetails currentMoveDetails = allMoves[(int)currentCombat - 1];

	currentAttackTime = currentMoveDetails.AT;
	currentEffectTime = currentMoveDetails.ET;
	currentEffetDistance = currentMoveDetails.ED;
	currentDmgAmount = currentMoveDetails.Dmg;
	currentDirection = currentMoveDetails.Dir;
	currentPower = currentMoveDetails.Power;
	currentHitPos = currentMoveDetails.Pos;
	currentHitTime = currentMoveDetails.HT;
}

// --------------------------------------------------------------------------------------------
// Sword Attack
void CombatManager::NextSwordCombat()
{
	if (currentCombat == Combat.none)
	{
		combatAudio.PlayOneShot(swordFX);
		currentCombat = Combat.Sword_Attack_R;
	}
	else if (currentCombat == Combat.Sword_Attack_R)
	{
		combatAudio.PlayOneShot(swordFX);
		currentCombat = Combat.Sword_Attack_RL;
	}
	else
	{
		combatAudio.PlayOneShot(swordFX);
		currentCombat = Combat.Sword_Attack_R;
	}

	// Reduce by 1 because of enum.
	CombatMoveDetails currentMoveDetails = allMoves[(int)currentCombat - 1];

	currentAttackTime = currentMoveDetails.AT;
	currentEffectTime = currentMoveDetails.ET;
	currentEffetDistance = currentMoveDetails.ED;
	currentDmgAmount = currentMoveDetails.Dmg;
	currentDirection = currentMoveDetails.Dir;
	currentPower = currentMoveDetails.Power;
	currentHitPos = currentMoveDetails.Pos;
	currentHitTime = currentMoveDetails.HT;

	sword.SetActive(true);
	compPos = companion.transform.position;
	companion.SetActive(false);
	swordGunAttack = true;
	attackDuration = currentAttackTime;
}

void CombatManager::NextSwordSpecial()
{
	if (currentCombat == Combat.none)
	{
		combatAudio.PlayOneShot(swordFX);
		currentCombat = Combat.Sword_Attack_Combo_LL;
	}
	else if (currentCombat == Combat.punch_Jab_L)
	{
		combatAudio.PlayOneShot(swordFX);
		currentCombat = Combat.Sword_Attack_Sp_U;
	}
	else if (currentCombat == Combat.punch_Jab_R)
	{
		combatAudio.PlayOneShot(swordFX);
		currentCombat = Combat.Sword_Attack_Sp_U;
	}
	else if (currentCombat == Combat.punch_Hook_L)
	{
		combatAudio.PlayOneShot(swordFX);
		currentCombat = Combat.Sword_Attack_Sp_U;
	}
	else if (currentCombat == Combat.punch_Hook_R)
	{
		combatAudio.PlayOneShot(swordFX);
		currentCombat = Combat.Sword_Attack_Sp_U;
	}
	else if (currentCombat == Combat.kick_Straight_Mid_R)
	{
		combatAudio.PlayOneShot(swordFX);
		currentCombat = Combat.Sword_Attack_Sp_U;
	}
	else if (currentCombat == Combat.Sword_Attack_RL)
	{
		combatAudio.PlayOneShot(swordFX);
		currentCombat = Combat.Sword_Attack_Sp_U;
	}
	else
	{
		combatAudio.PlayOneShot(swordFX);
		currentCombat = Combat.Sword_Attack_Combo_LL;
	}

	// 
	CombatMoveDetails currentMoveDetails = allMoves[(int)currentCombat - 1];

	currentAttackTime = currentMoveDetails.AT;
	currentEffectTime = currentMoveDetails.ET;
	currentEffetDistance = currentMoveDetails.ED;
	currentDmgAmount = currentMoveDetails.Dmg;
	currentDirection = currentMoveDetails.Dir;
	currentPower = currentMoveDetails.Power;
	currentHitPos = currentMoveDetails.Pos;
	currentHitTime = currentMoveDetails.HT;

	sword.SetActive(true);
	compPos = companion.transform.position;
	companion.SetActive(false);
	swordGunAttack = true;
	attackDuration = currentAttackTime;
}
// --------------------------------------------------------------------------------------------

/*
IEnumerator DelayBeforeDamage(float delay, float dmg)
{
	yield return new WaitForSeconds(delay);

	this.GetComponent<Humanoid>().TakeDamag(dmg);
}*/

// Setters
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