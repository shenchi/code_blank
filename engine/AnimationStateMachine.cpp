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
		cache = new AnimationStateCache();
		cache->frameCaches.resize(model->header->NumBones);
	}

	void AnimationStateCache::Reset()
	{
		ticks = 0;
		cursor = 0;

		for (AnimationFrameCache cache : frameCaches) {
			cache.Reset();
		}
	}

	void AnimationStateCache::Update(UpdateContext* context, ModelAnimation* animation)
	{
		// TODO: backward update cache

		// prevent load-hit-store
		size_t tempCursor = cursor;

		while (tempCursor < animation->numFrames) {
			size_t frameIndex = tempCursor + animation->startFrames;
			ModelAnimFrame &frame = context->model->frames[frameIndex];
			AnimationFrameCache &cache = frameCaches[frame.GetJointIndex()];

			size_t cacheIndex = cache.indices[frame.GetChannelType()][2];

			if (cacheIndex == SIZE_MAX || context->model->frames[cacheIndex].time <= ticks) {
				cache.AddFrameIndex(frame.GetChannelType(), frameIndex);
				tempCursor++;
			}
			else {
				break;
			}
		}
		cursor = tempCursor;
	}

	AnimNodeBase::AnimNodeBase(std::string name)
		:name(name)
	{
	}

	void AnimationState::Exit()
	{
		free(cache);
	}

	void AnimationState::Update(UpdateContext& context)
	{
		model::ModelAnimation *anim = context.model->GetAnimation(animationName);

		// TODO: scale time || uint_16 ticks
		// convert time in seconds to ticks
		cache->ticks += Time::DeltaTime * playbackSpeed * anim->ticksPerSecond;

		if (cache->ticks > anim->tickCount) {
			if (isLoop) {
				cache->ticks = std::fmodf(cache->ticks, anim->tickCount);
				cache->Reset();
			}
			else {
				// end of animation
				// event?
				// Transition?
			}
		}

		cache->Update(&context, anim);
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

	float AnimationState::GetDuration(Model * model)
	{
		auto anim = model->GetAnimation(animationName);
		return anim->tickCount * anim->ticksPerSecond;
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
	
	// AnimationStateMachine
	
	AnimationStateMachine::AnimationStateMachine(std::string name) : AnimNodeBase(name)
	{
		states.push_back(new AnimNodeBase("entry"));
		current = states.back();
	}

	AnimationStateMachine::~AnimationStateMachine()
	{
		for (AnimNodeBase* node : states) {
			delete node;
		}
	}

	AnimationState* AnimationStateMachine::AddState(std::string name)
	{
		stateIndexTable[name] = static_cast<uint16_t>(states.size());
		states.push_back(new AnimationState(name));

		return static_cast<AnimationState*>(states.back());
	}

	void AnimationStateMachine::Play(std::string name)
	{
		transitions.push_front(AnimationTransitionEntry{ name, 0.0f });
	}

	void AnimationStateMachine::CrossFade(std::string name, float normalizedDuration)
	{
		transitions.push_front(AnimationTransitionEntry{ name, normalizedDuration });
	}

	void AnimationStateMachine::Enter(Model *model)
	{
	
	}

	void AnimationStateMachine::Exit()
	{
		if (previous) {
			previous->Exit();
		}
		
		current->Exit();
	}

	void AnimationStateMachine::Update(UpdateContext& context)
	{
		// check transition
		for (AnimationTransitionEntry &entry : transitions) {

			if (stateIndexTable.find(entry.name) != stateIndexTable.end()) {

				if (previous) {
					previous->Exit();
				}
				previous = current;

				current = states[stateIndexTable[entry.name]];
				current->Enter(context.model);

				transitionDuration = entry.normalizedDuration * current->GetDuration(context.model);
				break;
			}
		}

		transitions.clear();
		
		// update current animation play back time
		elapsedTime += Time::DeltaTime;

		current->Update(context);

		if (transitionDuration)
			previous->Update(context);
	}

	void AnimationStateMachine::Evaluate(EvaluateContext & context)
	{


		current->Evaluate(context);
	}

	float AnimationStateMachine::GetDuration(Model * model)
	{
		return current->GetDuration(model);
	}

	/*void TransitionState::Enter(Model * model)
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
		
	}*/
}

