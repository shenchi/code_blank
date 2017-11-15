#include "AnimationComponent.h"

#include "TofuMath.h"
#include "Transform.h"
#include "RenderingComponent.h"

#include <cassert>

namespace tofu
{	

	int32_t AnimationComponentData::FillInBoneMatrices(void* buffer, uint32_t bufferSize, uint32_t animId, float time)
	{
		math::float4x4* matrices = reinterpret_cast<math::float4x4*>(buffer);

		RenderingComponent renderable = entity.GetComponent<RenderingComponent>();
		
		if (nullptr == model)
		{
			return TF_UNKNOWN_ERR;
		}

		if (animId >= model->header->NumAnimations || 
			static_cast<uint32_t>(sizeof(math::float4x4)) * model->header->NumBones > bufferSize)
		{
			return TF_UNKNOWN_ERR;
		}

		model::ModelAnimation& anim = model->animations[animId];

		float ticks = time * anim.ticksPerSecond; 
		ticks = std::fmodf(ticks, anim.durationInTicks);

		for (uint32_t i = 0; i < model->header->NumBones; i++)
		{
			matrices[i] = model->bones[i].transform;
		}

		for (uint32_t i = 0; i < anim.numChannels; i++)
		{
			model::ModelAnimChannel& chan = model->channels[anim.startChannelId + i];

			uint32_t boneId = chan.boneId;
			
			Transform t;
			t.SetTranslation(SampleFrame(model->translationFrames, chan.startTranslationFrame, chan.numTranslationFrame, ticks));
			t.SetRotation(SampleFrame(model->rotationFrames, chan.startRotationFrame, chan.numRotationFrame, ticks));
			t.SetScale(SampleFrame(model->scaleFrames, chan.startScaleFrame, chan.numScaleFrame, ticks));

			matrices[boneId] = t.GetMatrix();
		}

		for (uint32_t i = 0; i < model->header->NumBones; i++)
		{
			uint32_t p = model->bones[i].parent;
			if (UINT32_MAX != p)
			{
				matrices[i] = matrices[p] * matrices[i];
			}
		}

		for (uint32_t i = 0; i < model->header->NumBones; i++)
		{
			matrices[i] = matrices[i] * model->bones[i].offsetMatrix;
		}

		return TF_OK;
	}

	math::float3 AnimationComponentData::SampleFrame(model::ModelFloat3Frame* frames, uint32_t startFrame, uint32_t numFrames, float ticks)
	{
		if (nullptr != frames && numFrames > 0)
		{
			if (numFrames < 2)
				return frames[startFrame].value;
			
			for (uint32_t i = 1, last = 0; i < numFrames; i++)
			{
				model::ModelFloat3Frame& fb = frames[startFrame + i];

				if (fb.time > ticks)
				{
					model::ModelFloat3Frame& fa = frames[startFrame + last];
					float t = (ticks - fa.time) / (fb.time - fa.time);
					assert(!std::isnan(t) && !std::isinf(t) && t >= 0.0f && t <= 1.0f);
					return math::lerp(fa.value, fb.value, t);
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
			if (numFrames < 2)
				return frames[startFrame].value;

			for (uint32_t i = 1, last = 0; i < numFrames; i++)
			{
				model::ModelQuatFrame& fb = frames[startFrame + i];

				if (fb.time > ticks)
				{
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
