#pragma once
#include "Companion.h"
#include "GameplayDS.h"
#include "Enemy.h"

class CombatManager
{
public:
	CombatManager(bool, void*);
	~CombatManager();

	void Update(float);

	
	// A List with all the Actions and their parameters. ( Dictionary has to made serializable manually by writing a new class and stuff ).
	// The index is the enum value of the combat. (The evaluated integer)  
	//List<CombatMoveDetails> allMoves;

private:

	//List<GameObject> enemyList;
	//GameObject[] enemyArray;
	bool isPlayer;
	//*******************************************************************

	//Character m_char;
	//AudioSource combatAudio;
	//AudioClip punchFX;
	//AudioClip kickFX;
	//AudioClip swordFX;
	//AudioClip gunShotFX;

	//---------------------------------------------------------------------------------------------
	// Special Combat Variables
	//GameObject gun;
	//GameObject sword;
	//GameObject wrist;
	Companion* companion;
	tofu::math::float3 compPos;

	Combat combat;

	//target parameters
	tofu::math::float3 currentTarget;

	tofu::math::float3 aimTarget;
	
	int moveDir;

	//in combat parameters
	bool inCombat;
	float inCombatTimer;

	float inCombatDuration;

	//aimming parameters
	bool isAimming;
	
	
	float maxShotDistance;
	float minShotDistance;

	//move parameters
	bool isMoving;
	bool isDashing;
	

	//jump parameters
	bool isJumping;

	float jumpUpTime;

	float jumpAirTime;

	float jumpDownTime;
	

	//attack parameters
	bool isAttacking;
	bool resetAttack;
	float attackDuration;
	float currentAttackTime;
	float currentEffectTime;
	float currentEffetDistance;
	float currentDmgAmount;
	float currentHitTime;
	CombatDirection currentDirection;
	HitPower currentPower;
	HitPosition currentHitPos;
	

	//dodge parameters
	bool isDodging;

	float dodgeTime;
	int dodgeDirection;
	


	//roll parameters
	bool isRolling;

	float rollTime;

	float rollSpeed;
	

	//hit parameters
	bool isHit;
	bool resetHit;

	float hitTime;

	float hitMaxWalkSpeed;
	int hitAnimationInfo;
	

	//adjust parameters
	bool isAdjusting;

	float adjustSpeed;

	float adjustMinDistance;
	

	float adjustMaxDistance;

	float adjustAgle;

	Combat currentCombat;
	
	float comboTimer;

	float maxComboTime;

	bool swordGunAttack;

	bool canAim;

	bool canShoot;

	int listCounter;
};

/* Make into functions

int MoveDir;

public Character CurrentTarget
{ get{ return currentTarget; } set{ currentTarget = value; } }

public Character AimTarget
{ get{ return aimTarget; } set{ aimTarget = value; } }

public bool InCombat
{ get{ return inCombat; } }

HitPosition CurrentHitPos;

public bool IsAimming
{
get{ return isAimming; }
set{ isAimming = value; }
}


public bool IsMoving
{ get{ return isMoving; }
set{ isMoving = value; }
}
public bool IsDashing
{ get{ return isDashing; }
set{ isDashing = value; }
}

public bool IsJumping
{
get{ return isJumping; }
set{ isJumping = value; }
}
public float JumpUpTime
{ get{ return jumpUpTime; } }
public float JumpAirTime
{ get{ return jumpAirTime; } }
public float JumpDownTime
{ get{ return jumpDownTime; } }

public bool IsAttacking
{
get{ return isAttacking; }
set{ isAttacking = value; }
}
public float CurrentAttackTime
{
get{ return currentAttackTime; }
}
public float CurrentEffectTime
{
get{ return currentEffectTime; }
}
public bool ResetAttack
{
get{ return resetAttack; }
set{ resetAttack = value; }
}

public bool IsDodging
{ get{ return isDodging; }
set{ isDodging = value; }
}
public float DodgeTime
{ get{ return dodgeTime; } }
public int DodgeDirection
{ get{ return dodgeDirection; } }

public bool IsRolling
{ get{ return isRolling; }
set{ isRolling = value; }
}
public float RollTime
{ get{ return rollTime; } }
public float RollSpeed
{ get{ return rollSpeed; } }

public bool IsHit
{
get{ return isHit; }
}
public float HitTime
{
get{ return hitTime; }
set{ hitTime = value; }
}

public bool IsAdjusting
{
get{ return isAdjusting; }
set{ isAdjusting = value; }
}
public float AdjustSpeed
{ get{ return adjustSpeed; } }

public float GetAdjustMinDistance()
{
return adjustMinDistance;
}

public float GetAdjustMaxDistance()
{
return adjustMaxDistance;
}

public float GetAdjustAngle()
{
return adjustAgle;
}

public Combat CurrentCombat
{ get{ return currentCombat; } }

public float ComboTimer
{
get{ return comboTimer; }
set{ comboTimer = value; }
}

public bool canMove
{
get
{
return !(isRolling || isHit || isAttacking || isDodging || isAdjusting);
}
}

public bool canJump
{
get
{
return !(isRolling || isJumping || isAimming || isAttacking || isDodging || isAdjusting);
}
}

public bool canAttack
{
get
{
return !(isRolling || isJumping || isAttacking || isDodging || isAdjusting);
}
}

public bool canSetNextAttack
{
get
{
return isAttacking;
}
}

public bool canDodge
{
get
{
return !(isRolling || isJumping || isAttacking || isDodging || isAdjusting);
}
}

public bool canRoll
{
get
{
return !(isRolling || isJumping || isAimming || isAttacking || isDodging || isAdjusting);
}
}
*/