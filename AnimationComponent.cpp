#include "AnimationComponent.h"

#include "TofuMath.h"
#include "Transform.h"
#include "RenderingComponent.h"

namespace tofu
{

	int32_t AnimationComponentData::FillInBoneMatrices(void* buffer, uint32_t bufferSize, uint32_t animId, float time)
	{
		math::float4x4* matrices = reinterpret_cast<math::float4x4*>(buffer);

		RenderingComponent renderable = entity.GetComponent<RenderingComponent>();
		
		if (model != renderable->GetModel())
		{
			model = renderable->GetModel();
		}

		if (nullptr == model || !model->HasAnimation())
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

		{
			model::ModelAnimChannel& chan = model->channels[0];

			uint32_t boneId = chan.boneId;
			
			Transform t;
			t.SetTranslation(SampleFrame(model->translationFrames, chan.startTranslationFrame, chan.numTranslationFrame, ticks));
			t.SetRotation(SampleFrame(model->rotationFrames, chan.startRotationFrame, chan.numRotationFrame, ticks));
			t.SetScale(SampleFrame(model->scaleFrames, chan.startScaleFrame, chan.numScaleFrame, ticks));

			matrices[boneId] = t.GetMatrix();
		}

		return TF_OK;
	}

	math::float3 AnimationComponentData::SampleFrame(model::ModelFloat3Frame* frames, uint32_t startFrame, uint32_t numFrames, float ticks)
	{
		if (nullptr != frames && numFrames > 0)
		{
			if (numFrames < 2)
				return frames[startFrame].value;

			for (uint32_t i = 0; i < numFrames - 1; i++)
			{
				model::ModelFloat3Frame& fa = frames[startFrame + i];

				if (fa.time <= ticks)
				{
					model::ModelFloat3Frame& fb = frames[startFrame + i + 1];
					float t = (ticks - fa.time) / (fb.time - fa.time);
					return math::lerp(fa.value, fb.value, t);
				}
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

			for (uint32_t i = 0; i < numFrames - 1; i++)
			{
				model::ModelQuatFrame& fa = frames[startFrame + i];

				if (fa.time <= ticks)
				{
					model::ModelQuatFrame& fb = frames[startFrame + i + 1];
					float t = (ticks - fa.time) / (fb.time - fa.time);
					return math::slerp(fa.value, fb.value, t);
				}
			}
		}
		return math::quat();
	}

}
