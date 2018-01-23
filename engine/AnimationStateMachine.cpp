#include "AnimationStateMachine.h"
#include "Engine.h"
#include "Model.h"

namespace tofu 
{
	void AnimNodeBase::Update(Model * model)
	{

	}
	void AnimNodeBase::Evaluate(Model * model, PoseContext & Output)
	{
		
	}

	void AnimationState::Enter()
	{
		cache = new AnimationStateCache();
	}

	void AnimationState::Exit()
	{
		free(cache);
	}

	void AnimationState::Update(Model * model)
	{
		model::ModelAnimation& anim = model->animations[model->GetAnimationIndex(animationName)];

		// TODO: scale time || uint_16 ticks
		// convert time in seconds to ticks
		cache->ticks += Time::DeltaTime * playbackSpeed * anim.ticksPerSecond;

		if (cache->ticks > anim.tickCount - 1.f) {
			if (isLoop) {
				cache->ticks = std::fmodf(cache->ticks, anim.tickCount - 1.f);
				cache->Reset();
			}
			else {
				// end of animation
				// event?
				// Transition?
			}
		}
	}

	void AnimationState::Evaluate(Model * model, PoseContext & Output)
	{
	}

	
	void AnimationStateMachine::AddState(AnimationState & state)
	{
		stateIndexTable[state.name] = states.size();
		states.push_back(std::move(state));
	}

	void AnimationStateMachine::SetStartState(std::string name)
	{
		startState = stateIndexTable[name];
	}

	void AnimationStateMachine::Enter()
	{
		current = &states[startState];
		current->Enter();
	}

	void AnimationStateMachine::Exit()
	{
		current->Exit();
	}

	void AnimationStateMachine::Update(Model * model)
	{
		// update current animation play back time
		elapsedTime += Time::DeltaTime;
		
		// check transition

		// if no transition, update previous state
		current->Update(model);
	}

	void AnimationStateMachine::Evaluate(Model * model, PoseContext & Output)
	{

	}
	
	void AnimationStateCache::Reset()
	{
		cursor = 0;

		for (AnimationFrameCache cache : caches) {
			cache.Reset();
		}
	}
}

