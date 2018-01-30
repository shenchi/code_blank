#pragma once

#include "Component.h"
#include "RenderingSystem.h"
#include "AnimationStateMachine.h"

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
			stateMachine("default"),
			currentAnimation(0),
			currentTime(0.0f),
			playbackSpeed(1.0f),
			crossFadeFactor(0.0f),
			crossFadeSpeed(0.0f),
			lastAnimation(0)
		{}

		AnimationStateMachine* GetStateMachine() { return &stateMachine; }

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

		AnimationStateMachine	stateMachine;

		// id of animation currently used
		uint32_t				currentAnimation;
		float					currentTime;
		float					ticks;
		float					playbackSpeed;

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

		void UpdateStateMachine();

		// get interpolated vector frame for given ticks
		math::float3 SampleFrame(model::ModelFloat3Frame* frames, uint32_t startFrame, uint32_t numFrames, float ticks);

		// get interpolated quaterion frame for given ticks
		math::quat SampleFrame(model::ModelQuatFrame* frames, uint32_t startFrame, uint32_t numFrames, float ticks);

		void ReallocResources() {}
	};

	typedef Component<AnimationComponentData> AnimationComponent;
}
