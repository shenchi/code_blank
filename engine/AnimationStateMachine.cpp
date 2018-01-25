#include "AnimationStateMachine.h"
#include "Engine.h"
#include "Model.h"
#include "Transform.h"
#include "Compression.h"
#include <assert.h>
#include <algorithm>

using namespace tofu::model;

namespace tofu 
{
	void AnimationFrameCache::Reset()
	{
		// Assume channel numbers = 3
		for (int i = 0; i < 3; i++) {
			indices[i][0] = SIZE_MAX;
			indices[i][1] = SIZE_MAX;
			indices[i][2] = SIZE_MAX;
			indices[i][3] = SIZE_MAX;
		}
	}

	void AnimationFrameCache::AddFrameIndex(model::ChannelType type, size_t index)
	{
		indices[type][0] = indices[type][1];
		indices[type][1] = indices[type][2];
		indices[type][2] = indices[type][3];
		indices[type][3] = index;
	}

	void AnimationState::Enter(Model *model)
	{
		if (!cache) {
			cache = new AnimationStateCache();
			cache->frameCaches.resize(model->header->NumBones);
		}
	}

	void AnimationStateCache::Reset()
	{
		ticks = 0;
		cursor = 0;

		for (AnimationFrameCache cache : frameCaches) {
			cache.Reset();
		}
	}

	void AnimationState::Exit()
	{
		free(cache);
	}

	void AnimationState::Update(UpdateContext& context)
	{
		model::ModelAnimation& anim = context.model->animations[context.model->GetAnimationIndex(animationName)];

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

	void AnimationState::Evaluate(EvaluateContext & context)
	{
		for (size_t i = 0; i < cache->frameCaches.size(); i++)
		{
			AnimationFrameCache &frameCache = cache->frameCaches[i];

			if (frameCache.indices[kChannelTranslation][3] == SIZE_MAX &&
				frameCache.indices[kChannelRotation][3] == SIZE_MAX &&
				frameCache.indices[kChannelScale][3] == SIZE_MAX) {
				continue;
			}

			Transform trans;

			// TODO: Spline calculation 
			/*if (cache.indices[kChannelTranslation][0] != SIZE_MAX) {

			}
			else if (cache.indices[kChannelTranslation][1] != SIZE_MAX)*/ 

			if (frameCache.indices[kChannelTranslation][1] != SIZE_MAX
				&& context.model->frames[frameCache.indices[kChannelTranslation][1]].time <= cache->ticks) {

				trans.SetTranslation(
					LerpFromFrameIndex(
						context.model,
						frameCache.indices[kChannelTranslation][1],
						frameCache.indices[kChannelTranslation][2]
					));
			}
			else if (frameCache.indices[kChannelTranslation][2] != SIZE_MAX) {
				trans.SetTranslation(
					LerpFromFrameIndex(
						context.model,
						frameCache.indices[kChannelTranslation][2],
						frameCache.indices[kChannelTranslation][3]
					));
			}
			else if (frameCache.indices[kChannelTranslation][3] != SIZE_MAX) {
				trans.SetTranslation(context.model->frames[frameCache.indices[kChannelTranslation][3]].value);
			}

			// Rotation
			if (frameCache.indices[kChannelRotation][1] != SIZE_MAX
				&& context.model->frames[frameCache.indices[kChannelRotation][1]].time <= cache->ticks) {

				trans.SetRotation(
					SlerpFromFrameIndex(
						context.model,
						frameCache.indices[kChannelRotation][1],
						frameCache.indices[kChannelRotation][2]
					));

			}
			else if (frameCache.indices[kChannelRotation][2] != SIZE_MAX) {
				trans.SetRotation(
					SlerpFromFrameIndex(
						context.model,
						frameCache.indices[kChannelRotation][2],
						frameCache.indices[kChannelRotation][3]
					));

			}
			else if (frameCache.indices[kChannelRotation][3] != SIZE_MAX) {
				math::quat q;
				math::float3 &compress = context.model->frames[frameCache.indices[kChannelTranslation][3]].value;

				tofu::compression::DecompressQuaternion(*reinterpret_cast<uint32_t*>(&compress.x), q);

				trans.SetRotation(q);
			}

			// Scale
			if (frameCache.indices[kChannelScale][1] != SIZE_MAX
				&& context.model->frames[frameCache.indices[kChannelScale][1]].time <= cache->ticks) {

				trans.SetScale(
					LerpFromFrameIndex(
						context.model,
						frameCache.indices[kChannelScale][1],
						frameCache.indices[kChannelScale][2]
					));
			}
			else if (frameCache.indices[kChannelScale][2] != SIZE_MAX) {
				trans.SetScale(
					LerpFromFrameIndex(
						context.model,
						frameCache.indices[kChannelScale][2],
						frameCache.indices[kChannelScale][3]
					));
			}
			else if (frameCache.indices[kChannelScale][3] != SIZE_MAX) {
				trans.SetScale(context.model->frames[frameCache.indices[kChannelScale][3]].value);
			}

			context.Transformations[i] = trans.GetMatrix();
		}
	}

	math::float3 AnimationState::LerpFromFrameIndex(Model *model, size_t lhs, size_t rhs) const
	{
		ModelAnimFrame& fa = model->frames[lhs];
		ModelAnimFrame& fb = model->frames[rhs];

		float t = (cache->ticks - fa.time) / (fb.time - fa.time);
		assert(!std::isnan(t) && !std::isinf(t) && t >= 0.0f && t <= 1.0f);

		return math::mix(fa.value, fb.value, t);
	}

	math::quat AnimationState::SlerpFromFrameIndex(Model *model, size_t lhs, size_t rhs) const
	{
		ModelAnimFrame& fa = model->frames[lhs];
		ModelAnimFrame& fb = model->frames[rhs];

		float t = (cache->ticks - fa.time) / (fb.time - fa.time);
		assert(!std::isnan(t) && !std::isinf(t) && t >= 0.0f && t <= 1.0f);

		math::quat a, b;

		tofu::compression::DecompressQuaternion(*reinterpret_cast<uint32_t*>(&fa.value.x), a);
		tofu::compression::DecompressQuaternion(*reinterpret_cast<uint32_t*>(&fb.value.x), b);

		return math::slerp(a, b, t);
	}
	
	AnimationState& AnimationStateMachine::AddState(std::string name)
	{
		stateIndexTable[name] = static_cast<uint16_t>(states.size());
		states.push_back(AnimationState(name));

		return states.back();
	}

	void AnimationStateMachine::SetStartState(AnimationState &state)
	{
		SetStartState(state.name);
	}

	void AnimationStateMachine::SetStartState(std::string name)
	{
		startState = stateIndexTable[name];

		// FIXME: test noly
		if (name.compare("idle") == 0) {
			startState = 0;
		}
		else if (name.compare("walk") == 0) {
			startState = 1;
		}
	}

	void AnimationStateMachine::Enter(Model *model)
	{
		current = &states[startState];
		current->Enter(model);
	}

	void AnimationStateMachine::Exit()
	{
		current->Exit();
	}

	void AnimationStateMachine::Update(UpdateContext& context)
	{
		// update current animation play back time
		elapsedTime += Time::DeltaTime;
		
		// check transition

		// if no transition, update previous state
		current->Update(context);
	}

	void AnimationStateMachine::Evaluate(EvaluateContext & context)
	{
		current->Evaluate(context);
	}

	void TransitionState::Enter(Model * model)
	{
		next->Enter(model);
	}

	void TransitionState::Exit()
	{
		previous->Exit();
	}

	void TransitionState::Update(UpdateContext & context)
	{
		elapsedTime = std::min(elapsedTime + Time::DeltaTime, duration);
		
	}

	void TransitionState::Evaluate(EvaluateContext & context)
	{
		float alpha = elapsedTime / duration;
		

	

	}
}

