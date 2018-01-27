#pragma once

#include "Component.h"
#include "RenderingSystem.h"

namespace tofu
{
	class AnimationFrameCache 
	{
		friend class AnimationComponentData;

	public:
		AnimationFrameCache() {
			Reset();
		}

	private:
		size_t indices[3][4];

	private:
		void Reset();
		void AddFrameIndex(model::ChannelType type, size_t index);
	};

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
			cursor(0u),
			caches(nullptr),
			crossFadeFactor(0.0f),
			crossFadeSpeed(0.0f),
			lastAnimation(0)
		{}

		// switch to an animation
		int32_t Play(uint32_t animId);

		// cross fade to an animation in 'duration' time
		int32_t CrossFade(uint32_t animId, float duration);

		// get Id of the animation that is currently used
		TF_INLINE uint32_t GetCurrentAnimationId() const { return currentAnimation; }

	private:
		Entity					entity;
		Model*					model;
		
		// constant buffer for bone matrices
		BufferHandle			boneMatricesBuffer;
		uint32_t				boneMatricesBufferSize;

		// id of animation currently used
		uint32_t				currentAnimation;
		float					currentTime;
		float					ticks;
		float					playbackSpeed;

		// current position in key frames (for linear scan)
		uint32_t				cursor;
		// cache to keep t-1 to t+2 key frame index from previous search
		AnimationFrameCache		*caches;

		// interpolation parameter for cross fading
		// 1 stands for fully using old animation, 0 stands for fully using new animation
		float					crossFadeFactor;
		// speed of crossFadeFactor decreasing,  automatically set by cross fade duration
		float					crossFadeSpeed;

		// id of the old animation for cross fading
		uint32_t				lastAnimation;
		// play back time of the old animation
		float					lastAnimationTime;

	private:
		// update play back time and cross fade parameters
		void UpdateTiming();

		// calculate bone matrices and fill in the buffer
		int32_t FillInBoneMatrices(void* buffer, uint32_t bufferSize);

		// get interpolated vector frame for given ticks
		math::float3 SampleFrame(model::ModelFloat3Frame* frames, uint32_t startFrame, uint32_t numFrames, float ticks);

		// get interpolated quaterion frame for given ticks
		math::quat SampleFrame(model::ModelQuatFrame* frames, uint32_t startFrame, uint32_t numFrames, float ticks);

		math::float3 CatmullRomIndex(size_t i1, size_t i2, size_t i3, size_t i4);

		math::quat SquadIndex(size_t i1, size_t i2, size_t i3, size_t i4);

		math::float3 LerpFromFrameIndex(size_t lhs, size_t rhs);

		math::quat SlerpFromFrameIndex(size_t lhs, size_t rhs);

		void ResetCaches();
		void UpdateCache();
	};

	typedef Component<AnimationComponentData> AnimationComponent;
}
