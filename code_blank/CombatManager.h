#pragma once
#include "Companion.h"
#include "GameplayDS.h"
#include "Enemy.h"
#include "Player.h"

class CombatManager
{
public:
	CombatManager(bool, void*, void*);
	~CombatManager();

	void Update(float);
	bool UpdateState(float, float);
	void Hit(HitPosition, HitDirection, HitPower, float, float, float, bool);
	void BasicCombo();
	void SpecialCombat();
	void GunShot();
	void SwordCombo();
	void SwordSpecialCombat();
	void Attack();
	void Shoot();
	void Effect();
	bool CheckTarget();
	bool CheckRangeTarget();
	void Adjust();
	void Dodge(int);
	void Roll();
	void AimMove(int);
	void PerformAction(Combat);
	void NextCombat();
	void NextSpecial();
	void NextSwordCombat();
	void NextSwordSpecial();

	// Setters
	void SetComboTimer(float);
	void SetAimTarget(Character*);
	void SetCurrentTarget(Character*);
	void SetCurrentHitPos(HitPosition);
	void SetHitTime(float);
	void SetIsAdjusting(bool);
	void SetIsAimming(bool);
	void SetIsAttacking(bool);
	void SetIsDodging(bool);
	void SetIsJumping(bool);
	void SetIsMoving(bool);
	void SetIsRolling(bool);
	void SetIsSprinting(bool);
	void SetMoveDir(int _moveDir);
	void SetResetAttack(bool);

	// Getters
	float GetAdjustAngle();
	float GetAdjustMinDistance();
	float GetAdjustMaxDistance();
	float GetAdjustSpeed();
	Character* GetAimTarget();
	bool GetCanAttack();
	bool GetCanDodge();
	bool GetCanJump();
	bool GetCanMove();
	bool GetCanRoll();
	bool GetCanSetNextAttack();
	float GetComboTimer();
	float GetCurrentAttackTime();
	Combat GetCurrentCombat();
	float GetCurrentEffectTime();
	HitPosition GetCurrentHitPos();
	Character* GetCurrentTarget();
	int GetDodgeDirection();
	float GetDodgeTime();
	float GetHitTime();
	bool GetInCombat();
	bool GetIsAdjusting();
	bool GetIsAimming();
	bool GetIsAttacking();
	bool GetIsDodging();
	bool GetIsHit();
	bool GetIsJumping();
	bool GetIsMoving();
	bool GetIsRolling();
	bool GetIsSprinting();
	float GetJumpAirTime();
	float GetJumpDownTime();
	float GetJumpUpTime();
	int GetMoveDir();
	bool GetResetAttack();
	float GetRollSpeed();
	float GetRollTime();
	
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
	Enemy* enemy;
	Player* player;
	Companion* companion;
	tofu::math::float3 compPos;

	Combat combat;

	//target parameters
	Character* currentTarget;

	Character* aimTarget;
	
	int moveDir;

	//in combat parameters
	bool inCombat;
	float inCombatTimer;

	float inCombatDuration;

	//aimming parameters
	bool isAimming;
	
	Combat currentCombat;

	float comboTimer;

	float maxComboTime;

	bool swordGunAttack;

	bool canAim;

	bool canShoot;

	int listCounter;	

	float maxShotDistance;
	float minShotDistance;

	//move parameters
	bool isMoving;
	bool isSprinting;
	

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
};