#pragma once


enum CharacterState
{
	kNoState,
	kIdleOutCombat,
	kIdleInCombat,
	kRun,
	kJumpUp,
	kJumpAir,
	kJumpDown,
	kAttack,
	kAdjustPosition,
	kHit,
	kDodge,
	kRoll,
	kDrawGun,
	kAimIdle,
	kShoot,
	kHolsterGun,
	kAimMove,
	kDead
};

enum HitPosition
{
	kHigh,
	kMid,
	kLow
};

enum HitDirection
{
	kForward,
	kBackward,
	kLeft,
	kRight
};

enum CombatDirection
{
	kForward,
	kLeft,
	kRight
};

enum HitPower
{
	kWeak,
	kPowerful,
	kKO
};

enum Combat
{
	kNone,
	kPunchJabL,
	kPunchJabR,
	kPunchHookL,
	kPunchHookR,
	kPunchUpperCutL,
	kPunchUpperCutR,
	kKickStraightMidR,
	kKickAxeKick,
	kKickHorseKick,
	kSwordAttackR,
	kSwordAttackRL,
	kSwordAttackSp_U,
	kSwordAttackComboLL,
	kGunShoot,
	kNumberOfItems
};

struct CombatMoveDetails
{
	// AT - Attack Time
	// ET - Effect Time
	// ED - Effect Distance
	// HT - Hit Time
	// Dmg = Damage Amount
	// Name doesn't matter but is good for recognising.

	std::string name;
	float AT;
	float ET;
	float ED;
	float HT;
	float Dmg;
	HitPosition pos;
	CombatDirection dir;
	HitPower power;
};

struct CharacterDetails
{

};