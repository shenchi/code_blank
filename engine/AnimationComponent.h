#pragma once

#include "Component.h"
#include "RenderingSystem.h"
#include "AnimationStateMachine.h"

namespace tofu
{
	enum AvatarIKGoal {
		kIKGoalLeftFoot,
		kIKGoalRightFoot,
		kIKGoalLeftHand,
		kIKGoalRightHand
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
			boneMatricesBufferSize()
			{ layers.push_back(std::move(AnimationLayer("Base"))); }

		AnimationStateMachine* GetStateMachine(size_t layer = 0);

		// switch to an animation
		int32_t Play(std::string name, size_t layerIndex = 0);

		// cross fade to an animation in 'duration' time
		int32_t CrossFade(std::string name, float duration, size_t layerIndex = 0);

		bool SetIKPosition(AvatarIKGoal goal, math::float3 position);

	private:
		Entity					entity;
		Model*					model;

		// constant buffer for bone matrices
		BufferHandle			boneMatricesBuffer;
		uint32_t				boneMatricesBufferSize;

		std::vector<AnimationLayer> layers;

	private:
		// update play back time and cross fade parameters
		void UpdateTiming();

		// calculate bone matrices and fill in the buffer
		int32_t FillInBoneMatrices(void* buffer, uint32_t bufferSize);
	
		void ReallocResources() {}
	};

	typedef Component<AnimationComponentData> AnimationComponent;
}
