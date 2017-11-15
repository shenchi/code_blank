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
			playbackSpeed(1.0f)
		{}

		int32_t Play(uint32_t animId) 
		{ 
			currentAnimation = animId; 
			currentTime = 0.0f; 
			return TF_OK; 
		}

	private:
		Entity					entity;
		Model*					model;
		BufferHandle			boneMatricesBuffer;
		uint32_t				boneMatricesBufferSize;

		uint32_t				currentAnimation;
		float					currentTime;
		float					playbackSpeed;

	private:
		int32_t FillInBoneMatrices(void* buffer, uint32_t bufferSize, uint32_t animId, float time);

		math::float3 SampleFrame(model::ModelFloat3Frame* frames, uint32_t startFrame, uint32_t numFrames, float ticks);

		math::quat SampleFrame(model::ModelQuatFrame* frames, uint32_t startFrame, uint32_t numFrames, float ticks);
	};

	typedef Component<AnimationComponentData> AnimationComponent;
}
