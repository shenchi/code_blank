#pragma once

#include "Component.h"
#include "RenderingSystem.h"

namespace tofu
{
	class AnimationComponentData
	{
		friend class RenderingSystem;
	public:

		AnimationComponentData() : AnimationComponentData(Entity()) {}
		AnimationComponentData(Entity e)
			:
			entity(e),
			model(nullptr),
			boneMatricesBuffer(),
			boneMatricesBufferSize()
		{}

	private:
		Entity					entity;
		Model*					model;
		BufferHandle			boneMatricesBuffer;
		uint32_t				boneMatricesBufferSize;

		uint32_t				currentAnimation;

	private:
		int32_t FillInBoneMatrices(void* buffer, uint32_t bufferSize, uint32_t animId, float time);

		math::float3 SampleFrame(model::ModelFloat3Frame* frames, uint32_t startFrame, uint32_t numFrames, float ticks);

		math::quat SampleFrame(model::ModelQuatFrame* frames, uint32_t startFrame, uint32_t numFrames, float ticks);
	};

	typedef Component<AnimationComponentData> AnimationComponent;
}
