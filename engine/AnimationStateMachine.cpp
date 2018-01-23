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

	AnimationState::AnimationState()
	{
	}


	AnimationState::~AnimationState()
	{

	}

	void AnimationState::Update(Model * model)
	{
		uint16_t animIndex = model->animationTable[animationName];

		model::ModelAnimation& anim = model->animations[animIndex];

		if (cache) {
			// TODO: scale time || uint_16 ticks
			// convert time in seconds to ticks
			cache->ticks += Time::DeltaTime * playbackSpeed * anim.ticksPerSecond;

			if (cache->ticks > anim.tickCount - 1.f) {
				if (isLoop) {
					cache->ticks = std::fmodf(cache->ticks, anim.tickCount - 1.f);
					cache->caches.ResetCaches();
				}
				else {
					// end of animation
					// event?
				}
			}
		}

		
	}

	void AnimationState::Evaluate(Model * model, PoseContext & Output)
	{
	}

	
	void AnimationStateMachine::Update(Model * model)
	{
		// update current animation play back time
		elapsedTime += Time::DeltaTime * playbackSpeed;

		model::ModelAnimation& anim = model->animations[currentAnimation];

		// TODO: scale time || uint_16 ticks
		// convert time in seconds to ticks
		ticks = currentTime * anim.ticksPerSecond;

		// TODO: Add loop to animation
		bool loop = true;

		if (ticks > anim.tickCount - 1.f) {
			if (loop) {
				ticks = std::fmodf(ticks, anim.tickCount - 1.f);
				ResetCaches();
			}
			else {
				// end of animation
				// event?
			}
		}
	}

	void AnimationStateMachine::Evaluate(Model * model, PoseContext & Output)
	{

	}
	
}

