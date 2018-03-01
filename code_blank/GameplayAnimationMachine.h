#pragma once
#include <AnimationComponent.h>
#include "GameplayDS.h"

class CombatManager;

class GameplayAnimationMachine
{
public:
	GameplayAnimationMachine(CombatManager*);
	~GameplayAnimationMachine();

	void Play(CharacterState, uint32_t, size_t);
	void SetAnimComp(tofu::AnimationComponent);

private:
	tofu::AnimationComponent	aComp;
	CombatManager*				combatManager;
};