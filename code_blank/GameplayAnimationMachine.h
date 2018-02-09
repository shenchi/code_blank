#pragma once
#include <AnimationComponent.h>
#include "GameplayDS.h"

class CombatManager;

class GameplayAnimationMachine
{
public:
	GameplayAnimationMachine(tofu::AnimationComponent, CombatManager*);
	~GameplayAnimationMachine();

	void Play(CharacterState, uint32_t, size_t);

private:
	tofu::AnimationComponent	aComp;
	CombatManager*				combatManager;
};