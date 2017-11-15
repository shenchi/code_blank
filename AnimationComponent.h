#pragma once

#include "Component.h"
#include "RenderingSystem.h"

namespace tofu
{
	class AnimationComponentData
	{
		friend class RenderingSystem;
	public:

		TF_INLINE AnimationComponentData() : AnimationComponentData(Entity()) {}
		TF_INLINE AnimationComponentData(Entity e)
			:
			entity(e),
			model(nullptr),
			boneMatricesBuffer(),
			boneMatricesBufferSize(),
			currentAnimation(0),
			currentTime(0.0f),
			playbackSpeed(1.0f),
			crossFadeFactor(0.0f),
			crossFadeSpeed(0.0f),
			lastAnimation(0)
		{}

		int32_t Play(uint32_t animId);

		int32_t CrossFade(uint32_t animId, float duration);

		TF_INLINE uint32_t GetCurrentAnimationId() const { return currentAnimation; }

	private:
		Entity					entity;
		Model*					model;
		BufferHandle			boneMatricesBuffer;
		uint32_t				boneMatricesBufferSize;

		uint32_t				currentAnimation;
		float					currentTime;
		float					playbackSpeed;

		float					crossFadeFactor;
		float					crossFadeSpeed;
		uint32_t				lastAnimation;
		float					lastAnimationTime;

	private:
		void UpdateTiming();

		int32_t FillInBoneMatrices(void* buffer, uint32_t bufferSize);

		math::float3 SampleFrame(model::ModelFloat3Frame* frames, uint32_t startFrame, uint32_t numFrames, float ticks);

		math::quat SampleFrame(model::ModelQuatFrame* frames, uint32_t startFrame, uint32_t numFrames, float ticks);
	};

	typedef Component<AnimationComponentData> AnimationComponent;
}
