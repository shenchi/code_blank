#include "Transform.h"
#include "AnimationComponent.h"
#include "Engine.h"
#include "TofuMath.h"
#include "RenderingComponent.h"
#include <algorithm>
#include <cassert>

using namespace tofu::model;

namespace tofu
{
	int32_t AnimationComponentData::Play(uint32_t animId)
	{
		//if (animId != currentAnimation)
		//{
		//	// cancel cross fading if there is one
		//	crossFadeFactor = 0.0f;

		//	currentAnimation = animId;
		//	currentTime = 0.0f;
		//}

		if (animId == 0)
			stateMachine.Play("idle");
		else
			stateMachine.Play("walk");

		return kOK;
	}

	int32_t AnimationComponentData::CrossFade(uint32_t animId, float duration)
	{
		//// test if we are cross fading or trying to switch to the same animation
		//if (crossFadeFactor > 0.0f || animId == currentAnimation)
		//	return kOK;

		//// record old animation and set new animation
		//lastAnimation = currentAnimation;
		//currentAnimation = animId;

		//lastAnimationTime = currentTime;
		//currentTime = 0.0f;
		////ResetCaches();

		//// start cross fading, set cross fading speed
		//crossFadeFactor = 1.0f;
		//crossFadeSpeed = 1.0f / duration;

		if (animId == 0)
			stateMachine.CrossFade("idle", duration);
		else
			stateMachine.CrossFade("walk", duration);

		return kOK;
	}

	void AnimationComponentData::UpdateTiming()
	{
		UpdateStateMachine();

		//if (crossFadeFactor > 0.0f)
		//{
		//	// update cross fading and old animation parameters
		//	crossFadeFactor -= crossFadeSpeed * Time::DeltaTime;
		//	lastAnimationTime += Time::DeltaTime * playbackSpeed;
		//}

		//// update current animation play back time
		//currentTime += Time::DeltaTime * playbackSpeed;

		//model::ModelAnimation& anim = model->animations[currentAnimation];

		//// TODO: scale time || uint_16 ticks
		//// convert time in seconds to ticks
		//ticks = currentTime * anim.ticksPerSecond;
	}

	int32_t AnimationComponentData::FillInBoneMatrices(void* buffer, uint32_t bufferSize)
	{
		math::float4x4* matrices = reinterpret_cast<math::float4x4*>(buffer);

		if (nullptr == model)
		{
			return kErrUnknown;
		}

		if (currentAnimation >= model->header->NumAnimations ||
			static_cast<uint32_t>(sizeof(math::float4x4)) * model->header->NumBones > bufferSize)
		{
			return kErrUnknown;
		}

		// load bone matrices
		for (uint16_t i = 0; i < model->header->NumBones; i++)
		{
			matrices[i] = model->bones[i].transform;
		}

		EvaluateContext context(model);
		stateMachine.Evaluate(context, 1.0f);

		for (auto i = 0; i < model->header->NumBones; i++) {
			if (context.transforms[i].isDirty) {
				matrices[i] = context.transforms[i].GetMatrix();
			}
		}

		//model::ModelAnimation& anim = model->animations[currentAnimation];

		//// update bone matrices for each channel
		//for (uint32_t i = 0; i < anim.numChannels; i++)
		//{
		//	model::ModelAnimChannel& chan = model->channels[anim.startChannelId + i];

		//	uint16_t boneId = chan.boneId;

		//	// get interlopated matrix
		//	Transform t;
		//	t.SetTranslation(SampleFrame(model->translationFrames, chan.startTranslationFrame, chan.numTranslationFrame, ticks));
		//	t.SetRotation(SampleFrame(model->rotationFrames, chan.startRotationFrame, chan.numRotationFrame, ticks));
		//	t.SetScale(SampleFrame(model->scaleFrames, chan.startScaleFrame, chan.numScaleFrame, ticks));

		//	matrices[boneId] = t.GetMatrix();
		//}

		//// if we are cross fading
		//if (crossFadeFactor > 0.0f)
		//{
		//	model::ModelAnimation& lastAnim = model->animations[lastAnimation];

		//	float lastAnimTicks = currentTime * lastAnim.ticksPerSecond;
		//	lastAnimTicks = std::fmodf(lastAnimTicks, lastAnim.tickCount);

		//	// interplotate matrices between new and old animtion
		//	for (uint32_t i = 0; i < lastAnim.numChannels; i++)
		//	{
		//		model::ModelAnimChannel& chan = model->channels[lastAnim.startChannelId + i];

		//		uint16_t boneId = chan.boneId;

		//		Transform t;
		//		t.SetTranslation(SampleFrame(model->translationFrames, chan.startTranslationFrame, chan.numTranslationFrame, lastAnimTicks));
		//		t.SetRotation(SampleFrame(model->rotationFrames, chan.startRotationFrame, chan.numRotationFrame, lastAnimTicks));
		//		t.SetScale(SampleFrame(model->scaleFrames, chan.startScaleFrame, chan.numScaleFrame, lastAnimTicks));

		//		math::float4x4 m = t.GetMatrix();
		//		matrices[boneId] = math::mix(matrices[boneId], m, crossFadeFactor);
		//	}
		//}

		// convert to world space 
		for (uint16_t i = 1; i < model->header->NumBones; i++)
		{
			uint16_t p = model->bones[i].parent;
			matrices[i] = matrices[p] * matrices[i];
		}

		// append the offset matrices ( convert vertices from model space to bone local space )
		for (uint16_t i = 0; i < model->header->NumBones; i++)
		{
			float* p = reinterpret_cast<float*>(&(model->bones[i].offsetMatrix));

#ifdef TOFU_USE_GLM
			matrices[i] = math::transpose(matrices[i] * model->bones[i].offsetMatrix);
#else
			matrices[i] = matrices[i] * model->bones[i].offsetMatrix;
#endif
		}

		return kOK;
	}

	void AnimationComponentData::UpdateStateMachine()
	{
		// FIXME: Transition
		UpdateContext context{ model };
		stateMachine.Update(context);
	}

	math::float3 AnimationComponentData::SampleFrame(model::ModelFloat3Frame* frames, uint32_t startFrame, uint32_t numFrames, float ticks)
	{
		if (nullptr != frames && numFrames > 0)
		{
			// if we have only 1 frame ...
			if (numFrames < 2)
				return frames[startFrame].value;

			// find the 2 consecutive frames we are in between
			for (uint32_t i = 1, last = 0; i < numFrames; i++)
			{
				model::ModelFloat3Frame& fb = frames[startFrame + i];

				if (fb.time > ticks)
				{
					// lerp between these 2 frames
					model::ModelFloat3Frame& fa = frames[startFrame + last];
					float t = (ticks - fa.time) / (fb.time - fa.time);
					assert(!std::isnan(t) && !std::isinf(t) && t >= 0.0f && t <= 1.0f);
					return math::mix(fa.value, fb.value, t);
				}
				last = i;
			}
		}
		return math::float3();
	}

	math::quat AnimationComponentData::SampleFrame(model::ModelQuatFrame* frames, uint32_t startFrame, uint32_t numFrames, float ticks)
	{
		if (nullptr != frames && numFrames > 0)
		{
			// if we have only 1 frame ...
			if (numFrames < 2)
				return frames[startFrame].value;

			// find the 2 consecutive frames we are in between
			for (uint32_t i = 1, last = 0; i < numFrames; i++)
			{
				model::ModelQuatFrame& fb = frames[startFrame + i];

				if (fb.time > ticks)
				{
					// slerp between these 2 frames
					model::ModelQuatFrame& fa = frames[startFrame + last];
					float t = (ticks - fa.time) / (fb.time - fa.time);
					assert(!std::isnan(t) && !std::isinf(t) && t >= 0.0f && t <= 1.0f);
					return math::slerp(fa.value, fb.value, t);
				}
				last = i;
			}
		}
		return math::quat();
	}
}
