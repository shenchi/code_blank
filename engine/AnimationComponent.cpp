#include "AnimationComponent.h"

#include "Engine.h"
#include "TofuMath.h"
#include "Transform.h"
#include "RenderingComponent.h"
#include <algorithm>
#include "Compression.h"

#include <cassert>

using namespace tofu::model;

namespace tofu
{
	int32_t AnimationComponentData::Play(uint32_t animId)
	{
		if (animId != currentAnimation)
		{
			// cancel cross fading if there is one
			crossFadeFactor = 0.0f;

			currentAnimation = animId;
			currentTime = 0.0f;
		}
		return kOK;
	}

	int32_t AnimationComponentData::CrossFade(uint32_t animId, float duration)
	{
		// test if we are cross fading or trying to switch to the same animation
		if (crossFadeFactor > 0.0f || animId == currentAnimation)
			return kOK;

		// record old animation and set new animation
		lastAnimation = currentAnimation;
		currentAnimation = animId;

		lastAnimationTime = currentTime;
		currentTime = 0.0f;
		//ResetCaches();

		// start cross fading, set cross fading speed
		crossFadeFactor = 1.0f;
		crossFadeSpeed = 1.0f / duration;

		return kOK;
	}

	void AnimationComponentData::UpdateTiming()
	{
		if (crossFadeFactor > 0.0f)
		{
			// update cross fading and old animation parameters
			crossFadeFactor -= crossFadeSpeed * Time::DeltaTime;
			lastAnimationTime += Time::DeltaTime * playbackSpeed;
		}

		// update current animation play back time
		currentTime += Time::DeltaTime * playbackSpeed;

		model::ModelAnimation& anim = model->animations[currentAnimation];

		// TODO: scale time || uint_16 ticks
		// convert time in seconds to ticks
		ticks = currentTime * anim.ticksPerSecond;

		// TODO: Add loop to animation
		bool loop = true;

		if (ticks > anim.tickCount) {
			if (loop) {
				ticks = std::fmodf(ticks, anim.tickCount);
				//ResetCaches();
			}
			else {
				// end of animation
				// event?
			}
		}
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

		model::ModelAnimation& anim = model->animations[currentAnimation];
		
		// load bone matrices
		for (uint16_t i = 0; i < model->header->NumBones; i++)
		{
			matrices[i] = model->bones[i].transform;
		}

		// update bone matrices for each channel
		for (uint32_t i = 0; i < anim.numChannels; i++)
		{
			model::ModelAnimChannel& chan = model->channels[anim.startChannelId + i];

			uint16_t boneId = chan.boneId;

			// get interlopated matrix
			Transform t;
			t.SetTranslation(SampleFrame(model->translationFrames, chan.startTranslationFrame, chan.numTranslationFrame, ticks));
			t.SetRotation(SampleFrame(model->rotationFrames, chan.startRotationFrame, chan.numRotationFrame, ticks));
			t.SetScale(SampleFrame(model->scaleFrames, chan.startScaleFrame, chan.numScaleFrame, ticks));

			matrices[boneId] = t.GetMatrix();

			//traVector[boneId] = SampleFrame(model->translationFrames, chan.startTranslationFrame, chan.numTranslationFrame, ticks);
		}

		//UpdateCache();

		//for (uint16_t i = 0; i < model->header->NumBones; i++)
		//{
		//	AnimationFrameCache &cache = caches[i];

		//	if (cache.indices[kChannelTranslation][3] == SIZE_MAX &&
		//		cache.indices[kChannelRotation][3] == SIZE_MAX &&
		//		cache.indices[kChannelScale][3] == SIZE_MAX) {
		//		continue;
		//	}

		//	Transform trans;

		//	size_t *indices = cache.indices[kChannelTranslation];

		//	if (indices[1] != SIZE_MAX)
		//	{
		//		if (model->frames[indices[2]].time <= ticks) {
		//			trans.SetTranslation(CatmullRomIndex(indices[1], indices[2], indices[3], indices[3]));
		//		}
		//		else {
		//			trans.SetTranslation(CatmullRomIndex(indices[0] == SIZE_MAX ? indices[1] : indices[0], indices[1], indices[2], indices[3]));
		//		}
		//	}
		//	else if (indices[2] != SIZE_MAX) {
		//		trans.SetTranslation(LerpFromFrameIndex(indices[2], indices[3]));
		//	}
		//	else if (indices[3] != SIZE_MAX) {
		//		trans.SetTranslation(model->frames[indices[3]].value);
		//	}

		//	indices = cache.indices[kChannelRotation];

		//	if (indices[1] != SIZE_MAX)
		//	{
		//		if (model->frames[indices[2]].time <= ticks) {
		//			trans.SetRotation(SquadIndex(indices[1], indices[2], indices[3], indices[3]));
		//		}
		//		else {
		//			trans.SetRotation(SquadIndex(indices[0] == SIZE_MAX ? indices[1] : indices[0], indices[1], indices[2], indices[3]));
		//		}
		//	}
		//	else if (indices[2] != SIZE_MAX) {
		//		trans.SetRotation(SlerpFromFrameIndex(indices[2], indices[3]));
		//	}
		//	else if (indices[3] != SIZE_MAX) {
		//		math::quat q;
		//		math::float3 &compress = model->frames[cache.indices[kChannelTranslation][3]].value;

		//		tofu::compression::DecompressQuaternion(*reinterpret_cast<uint32_t*>(&compress.x), q);

		//		trans.SetRotation(q);
		//	}

		//	indices = cache.indices[kChannelScale];

		//	if (indices[1] != SIZE_MAX)
		//	{
		//		if (model->frames[indices[2]].time <= ticks) {
		//			trans.SetScale(CatmullRomIndex(indices[1], indices[2], indices[3], indices[3]));
		//		}
		//		else {
		//			trans.SetScale(CatmullRomIndex(indices[0] == SIZE_MAX ? indices[1] : indices[0], indices[1], indices[2], indices[3]));
		//			//trans.SetScale(LerpFromFrameIndex(indices[1], indices[2]));
		//		}
		//	}
		//	else if (indices[2] != SIZE_MAX) {
		//		trans.SetScale(LerpFromFrameIndex(indices[2], indices[3]));
		//	}
		//	else if (indices[3] != SIZE_MAX) {
		//		trans.SetScale(model->frames[indices[3]].value);
		//	}

		//	matrices[i] = trans.GetMatrix();
		//}

		// if we are cross fading
		if (crossFadeFactor > 0.0f)
		{
			model::ModelAnimation& lastAnim = model->animations[lastAnimation];

			float lastAnimTicks = currentTime * lastAnim.ticksPerSecond;
			lastAnimTicks = std::fmodf(lastAnimTicks, lastAnim.tickCount);

			// interplotate matrices between new and old animtion
			for (uint32_t i = 0; i < lastAnim.numChannels; i++)
			{
				model::ModelAnimChannel& chan = model->channels[lastAnim.startChannelId + i];

				uint16_t boneId = chan.boneId;

				Transform t;
				t.SetTranslation(SampleFrame(model->translationFrames, chan.startTranslationFrame, chan.numTranslationFrame, lastAnimTicks));
				t.SetRotation(SampleFrame(model->rotationFrames, chan.startRotationFrame, chan.numRotationFrame, lastAnimTicks));
				t.SetScale(SampleFrame(model->scaleFrames, chan.startScaleFrame, chan.numScaleFrame, lastAnimTicks));
				
				math::float4x4 m = t.GetMatrix();
				matrices[boneId] = math::mix(matrices[boneId], m, crossFadeFactor);
			}
		}

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

	math::float3 AnimationComponentData::CatmullRomIndex(size_t i1, size_t i2, size_t i3, size_t i4)
	{
		ModelAnimFrame& f1 = model->frames[i1];
		ModelAnimFrame& f2 = model->frames[i2];
		ModelAnimFrame& f3 = model->frames[i3];
		ModelAnimFrame& f4 = model->frames[i4];

		float t = (ticks - f2.time) / (f3.time - f2.time);
		assert(!std::isnan(t) && !std::isinf(t) && t >= 0.0f && t <= 1.0f);

		//return math::mix(f2.value, f3.value, t);
		return catmullRom(f1.value, f2.value, f3.value, f4.value, t);
	}

	math::quat AnimationComponentData::SquadIndex(size_t i1, size_t i2, size_t i3, size_t i4)
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

		float t = (ticks - f2.time) / (f3.time - f2.time);
		assert(!std::isnan(t) && !std::isinf(t) && t >= 0.0f && t <= 1.0f);

		//return math::slerp(q2, q3, t);
		return squad(q2, q3, intermediate(q1, q2, q3), intermediate(q2, q3, q4), t);
	}

	math::float3 AnimationComponentData::LerpFromFrameIndex(size_t lhs, size_t rhs)
	{
		ModelAnimFrame& fa = model->frames[lhs];
		ModelAnimFrame& fb = model->frames[rhs];

		float t = (ticks - fa.time) / (fb.time - fa.time);
		assert(!std::isnan(t) && !std::isinf(t) && t >= 0.0f && t <= 1.0f);

		return math::mix(fa.value, fb.value, t);
	}

	math::quat AnimationComponentData::SlerpFromFrameIndex(size_t lhs, size_t rhs)
	{
		ModelAnimFrame& fa = model->frames[lhs];
		ModelAnimFrame& fb = model->frames[rhs];

		float t = (ticks - fa.time) / (fb.time - fa.time);
		assert(!std::isnan(t) && !std::isinf(t) && t >= 0.0f && t <= 1.0f);

		math::quat a, b;

		tofu::compression::DecompressQuaternion(*reinterpret_cast<uint32_t*>(&fa.value.x), a);
		tofu::compression::DecompressQuaternion(*reinterpret_cast<uint32_t*>(&fb.value.x), b);

		return math::slerp(a, b, t);
	}

	void AnimationComponentData::UpdateCache()
	{
		// TODO: backward update cache
		if (cursor == model->animations[currentAnimation].numFrames) {
			return;
		}

		// prevent load-hit-store
		size_t tempCursor = cursor;

		std::vector<ModelAnimFrame> test;

		for (int i = 0; i <= model->header->NumAnimationFrames; i++) {
			test.push_back(model->frames[i]);
		}

		while (tempCursor < model->animations[currentAnimation].numFrames) {
			uint32_t frameIndex = tempCursor + model->animations[currentAnimation].startFrames;
			ModelAnimFrame &frame = model->frames[frameIndex];
			AnimationFrameCache &cache = caches[frame.GetJointIndex()];

			size_t cacheIndex = cache.indices[frame.GetChannelType()][2];

			if (cacheIndex == SIZE_MAX || model->frames[cacheIndex].time <= ticks) {
				cache.AddFrameIndex(frame.GetChannelType(), frameIndex);
				tempCursor++;
			}
			else {
				break;
			}
		}
		cursor = tempCursor;
	}

	void AnimationComponentData::ResetCaches() {
		cursor = 0;

		for (uint16_t i = 0; i < model->header->NumBones; i++) {
			caches[i] = AnimationFrameCache();
		}
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
}
