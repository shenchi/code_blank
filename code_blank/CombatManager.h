#pragma once
#include "Companion.h"
#include "GameplayDS.h"

class Character;
class Player;
class Enemy;

class CombatManager
{
public:
	CombatManager(bool, void*, void*);
	~CombatManager();

	void Update(float);
	bool UpdateState(float, float);
	void Hit(HitPosition, HitDirection, HitPower, float, float);
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
	void SetEnemyList(std::vector<Character*>*);

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
	std::vector<CombatMoveDetails> allMoves;

private:

	std::vector<Character*>* enemyList;
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
	Character* character;

	Companion* companion;
	tofu::math::float3 compPos;

	Combat combat;
	Combat currentCombat;

	// target parameters
	Character* currentTarget;
	Character* aimTarget;
	
	int moveDir;

	int listCounter;

	// in combat parameters
	bool inCombat;
	bool swordGunAttack;
	float inCombatTimer;
	float inCombatDuration;
	float comboTimer;
	float maxComboTime;
	

	// Aiming 
	bool isAimming;
	bool canAim;
	bool canShoot;
	float maxShotDistance;
	float minShotDistance;

	// move parameters
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