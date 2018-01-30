#include "Transform.h"
#include "AnimationStateMachine.h"
#include "Engine.h"
#include "Model.h"
#include "Compression.h"
#include <assert.h>
#include <algorithm>

using namespace tofu::model;

namespace tofu
{
	EvaluateContext::EvaluateContext(Model * model) :model(model)
	{
		transforms = new Transform[model->header->NumBones];
	}

	EvaluateContext::~EvaluateContext()
	{
		if (transforms)
			delete[] transforms;
	}

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

	void AnimationState::Exit()
	{
		delete cache;
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

	void AnimationState::Evaluate(EvaluateContext & context, float weight)
	{
		for (size_t i = 0; i < cache->frameCaches.size(); i++)
		{
			//AnimationFrameCache &frameCache = cache->frameCaches[i];

			//if (frameCache.indices[kChannelTranslation][3] == SIZE_MAX &&
			//	frameCache.indices[kChannelRotation][3] == SIZE_MAX &&
			//	frameCache.indices[kChannelScale][3] == SIZE_MAX) {
			//	continue;
			//}

			//Transform trans;

			//// TODO: Spline calculation 
			///*if (cache.indices[kChannelTranslation][0] != SIZE_MAX) {

			//}
			//else if (cache.indices[kChannelTranslation][1] != SIZE_MAX)*/ 

			//if (frameCache.indices[kChannelTranslation][1] != SIZE_MAX
			//	&& context.model->frames[frameCache.indices[kChannelTranslation][1]].time <= cache->ticks) {

			//	trans.SetTranslation(
			//		LerpFromFrameIndex(
			//			context.model,
			//			frameCache.indices[kChannelTranslation][1],
			//			frameCache.indices[kChannelTranslation][2]
			//		));
			//}
			//else if (frameCache.indices[kChannelTranslation][2] != SIZE_MAX) {
			//	trans.SetTranslation(
			//		LerpFromFrameIndex(
			//			context.model,
			//			frameCache.indices[kChannelTranslation][2],
			//			frameCache.indices[kChannelTranslation][3]
			//		));
			//}
			//else if (frameCache.indices[kChannelTranslation][3] != SIZE_MAX) {
			//	trans.SetTranslation(context.model->frames[frameCache.indices[kChannelTranslation][3]].value);
			//}

			//// Rotation
			//if (frameCache.indices[kChannelRotation][1] != SIZE_MAX
			//	&& context.model->frames[frameCache.indices[kChannelRotation][1]].time <= cache->ticks) {

			//	trans.SetRotation(
			//		SlerpFromFrameIndex(
			//			context.model,
			//			frameCache.indices[kChannelRotation][1],
			//			frameCache.indices[kChannelRotation][2]
			//		));

			//}
			//else if (frameCache.indices[kChannelRotation][2] != SIZE_MAX) {
			//	trans.SetRotation(
			//		SlerpFromFrameIndex(
			//			context.model,
			//			frameCache.indices[kChannelRotation][2],
			//			frameCache.indices[kChannelRotation][3]
			//		));

			//}
			//else if (frameCache.indices[kChannelRotation][3] != SIZE_MAX) {
			//	math::quat q;
			//	math::float3 &compress = context.model->frames[frameCache.indices[kChannelTranslation][3]].value;

			//	tofu::compression::DecompressQuaternion(*reinterpret_cast<uint32_t*>(&compress.x), q);

			//	trans.SetRotation(q);
			//}

			//// Scale
			//if (frameCache.indices[kChannelScale][1] != SIZE_MAX
			//	&& context.model->frames[frameCache.indices[kChannelScale][1]].time <= cache->ticks) {

			//	trans.SetScale(
			//		LerpFromFrameIndex(
			//			context.model,
			//			frameCache.indices[kChannelScale][1],
			//			frameCache.indices[kChannelScale][2]
			//		));
			//}
			//else if (frameCache.indices[kChannelScale][2] != SIZE_MAX) {
			//	trans.SetScale(
			//		LerpFromFrameIndex(
			//			context.model,
			//			frameCache.indices[kChannelScale][2],
			//			frameCache.indices[kChannelScale][3]
			//		));
			//}
			//else if (frameCache.indices[kChannelScale][3] != SIZE_MAX) {
			//	trans.SetScale(context.model->frames[frameCache.indices[kChannelScale][3]].value);
			//}

			AnimationFrameCache &frameCache = cache->frameCaches[i];

			if (frameCache.indices[kChannelTranslation][3] == SIZE_MAX &&
				frameCache.indices[kChannelRotation][3] == SIZE_MAX &&
				frameCache.indices[kChannelScale][3] == SIZE_MAX) {
				continue;
			}

			Model *model = context.model;

			Transform trans;

			size_t *indices = frameCache.indices[kChannelTranslation];

			if (indices[1] != SIZE_MAX)
			{
				if (model->frames[indices[2]].time <= cache->ticks) {
					trans.SetTranslation(CatmullRomIndex(model, indices[1], indices[2], indices[3], indices[3]));
				}
				else {
					trans.SetTranslation(CatmullRomIndex(model, indices[0] == SIZE_MAX ? indices[1] : indices[0], indices[1], indices[2], indices[3]));
				}
			}
			else if (indices[2] != SIZE_MAX) {
				trans.SetTranslation(LerpFromFrameIndex(model, indices[2], indices[3]));
			}
			else if (indices[3] != SIZE_MAX) {
				trans.SetTranslation(model->frames[indices[3]].value);
			}

			indices = frameCache.indices[kChannelRotation];

			if (indices[1] != SIZE_MAX)
			{
				if (model->frames[indices[2]].time <= cache->ticks) {
					trans.SetRotation(SquadIndex(model, indices[1], indices[2], indices[3], indices[3]));
				}
				else {
					trans.SetRotation(SquadIndex(model, indices[0] == SIZE_MAX ? indices[1] : indices[0], indices[1], indices[2], indices[3]));
				}
			}
			else if (indices[2] != SIZE_MAX) {
				trans.SetRotation(SlerpFromFrameIndex(model, indices[2], indices[3]));
			}
			else if (indices[3] != SIZE_MAX) {
				math::quat q;
				math::float3 &compress = model->frames[frameCache.indices[kChannelTranslation][3]].value;

				tofu::compression::DecompressQuaternion(*reinterpret_cast<uint32_t*>(&compress.x), q);

				trans.SetRotation(q);
			}

			indices = frameCache.indices[kChannelScale];

			if (indices[1] != SIZE_MAX)
			{
				if (model->frames[indices[2]].time <= cache->ticks) {
					trans.SetScale(CatmullRomIndex(model, indices[1], indices[2], indices[3], indices[3]));
				}
				else {
					trans.SetScale(CatmullRomIndex(model, indices[0] == SIZE_MAX ? indices[1] : indices[0], indices[1], indices[2], indices[3]));
					//trans.SetScale(LerpFromFrameIndex(indices[1], indices[2]));
				}
			}
			else if (indices[2] != SIZE_MAX) {
				trans.SetScale(LerpFromFrameIndex(model, indices[2], indices[3]));
			}
			else if (indices[3] != SIZE_MAX) {
				trans.SetScale(model->frames[indices[3]].value);
			}


			trans.isDirty = true;

			context.transforms[i].BlendByWeight(trans, weight);
		}
	}

	float AnimationState::GetDurationInSecond(Model * model)
	{
		auto anim = model->GetAnimation(animationName);
		return anim->tickCount / anim->ticksPerSecond;
	}

	math::float3 AnimationState::CatmullRomIndex(Model *model, size_t i1, size_t i2, size_t i3, size_t i4) const
	{
		ModelAnimFrame& f1 = model->frames[i1];
		ModelAnimFrame& f2 = model->frames[i2];
		ModelAnimFrame& f3 = model->frames[i3];
		ModelAnimFrame& f4 = model->frames[i4];

		float t = (cache->ticks - f2.time) / (f3.time - f2.time);
		assert(!std::isnan(t) && !std::isinf(t) && t >= 0.0f && t <= 1.0f);

		//return math::mix(f2.value, f3.value, t);
		return catmullRom(f1.value, f2.value, f3.value, f4.value, t);
	}

	math::quat AnimationState::SquadIndex(Model *model, size_t i1, size_t i2, size_t i3, size_t i4) const
	{
		ModelAnimFrame& f1 = model->frames[i1];
		ModelAnimFrame& f2 = model->frames[i2];
		ModelAnimFrame& f3 = model->frames[i3];
		ModelAnimFrame& f4 = model->frames[i4];

		math::quat q1, q2, q3, q4;

		/*tofu::compression::DecompressQuaternion(*reinterpret_cast<uint32_t*>(&f1.value.x), q1);
		tofu::compression::DecompressQuaternion(*reinterpret_cast<uint32_t*>(&f2.value.x), q2);
		tofu::compression::DecompressQuaternion(*reinterpret_cast<uint32_t*>(&f3.value.x), q3);
		tofu::compression::DecompressQuaternion(*reinterpret_cast<uint32_t*>(&f4.value.x), q4);*/

		tofu::compression::DecompressQuaternion(q1, f1.value, f1.GetSignedBit());
		tofu::compression::DecompressQuaternion(q2, f2.value, f2.GetSignedBit());
		tofu::compression::DecompressQuaternion(q3, f3.value, f3.GetSignedBit());
		tofu::compression::DecompressQuaternion(q4, f4.value, f4.GetSignedBit());

		float t = (cache->ticks - f2.time) / (f3.time - f2.time);
		assert(!std::isnan(t) && !std::isinf(t) && t >= 0.0f && t <= 1.0f);

		//return math::slerp(q2, q3, t);
		return squad(q2, q3, intermediate(q1, q2, q3), intermediate(q2, q3, q4), t);
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

			if (entry.name.compare(current->name) == 0)
				continue;

			if (stateIndexTable.find(entry.name) != stateIndexTable.end()) {

				if (previous) {
					previous->Exit();
				}
				previous = current;

				current = states[stateIndexTable[entry.name]];
				current->Enter(context.model);

				transitionDuration = entry.normalizedDuration * current->GetDurationInSecond(context.model);
				break;
			}
		}

		// update current animation play back time
		elapsedTime += Time::DeltaTime;

		if (transitionDuration) {
			if (elapsedTime > transitionDuration) {
				transitionDuration = 0.f;
			}
			else {
				previous->Update(context);
			}
		}

		// TODO: better way to prevent passing previous transition?
		transitions.clear();

		current->Update(context);
	}

	void AnimationStateMachine::Evaluate(EvaluateContext & context, float weight)
	{
		if (transitionDuration) {
			float alpha = elapsedTime / transitionDuration;

			if (weight == 1.0f) {
				// TODO: Prevent previous transition;
				previous->Evaluate(context, 1.0f);
				current->Evaluate(context, alpha);
			}
			else {
				EvaluateContext temp = context;
				temp.transforms = new Transform[context.model->header->NumBones];

				previous->Evaluate(temp, 1.0f);
				current->Evaluate(temp, alpha);

				for (auto i = 0; i < context.model->header->NumBones; i++) {
					context.transforms[i].BlendByWeight(temp.transforms[i], weight);
				}

				delete[] temp.transforms;
			}
		}
		else {
			current->Evaluate(context, weight);
		}
	}

	float AnimationStateMachine::GetDurationInSecond(Model * model)
	{
		return current->GetDurationInSecond(model);
	}
}

