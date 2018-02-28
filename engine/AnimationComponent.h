#pragma once

#include "Component.h"
#include "RenderingSystem.h"
#include "AnimationStateMachine.h"

namespace tofu
{
	struct IKTarget {
		// TODO: combine with HumanBoneId?
		enum HumanBoneID {
			None = -1,
			Root,
			Hips,
			Spine,
			Spine1,
			Spine2,
			Neck,
			Head,
			HeadTop,
			LeftShoulder,
			LeftArm,
			LeftForeArm,
			LeftHand,
			LeftHandThumb1,
			LeftHandThumb2,
			LeftHandThumb3,
			LeftHandThumb4,
			LeftHandIndex1,
			LeftHandIndex2,
			LeftHandIndex3,
			LeftHandIndex4,
			LeftHandMiddle1,
			LeftHandMiddle2,
			LeftHandMiddle3,
			LeftHandMiddle4,
			LeftHandRing1,
			LeftHandRing2,
			LeftHandRing3,
			LeftHandRing4,
			LeftHandPinky1,
			LeftHandPinky2,
			LeftHandPinky3,
			LeftHandPinky4,
			RightShoulder,
			RightArm,
			RightForeArm,
			RightHand,
			RightHandThumb1,
			RightHandThumb2,
			RightHandThumb3,
			RightHandThumb4,
			RightHandIndex1,
			RightHandIndex2,
			RightHandIndex3,
			RightHandIndex4,
			RightHandMiddle1,
			RightHandMiddle2,
			RightHandMiddle3,
			RightHandMiddle4,
			RightHandRing1,
			RightHandRing2,
			RightHandRing3,
			RightHandRing4,
			RightHandPinky1,
			RightHandPinky2,
			RightHandPinky3,
			RightHandPinky4,
			LeftUpLeg,
			LeftLeg,
			LeftFoot,
			LeftToeBase,
			LeftToeBaseEnd,
			RightUpLeg,
			RightLeg,
			RightFoot,
			RightToeBase,
			RightToeBaseEnd,
			NumHumanBones
		};

		HumanBoneID tipJoint;
		uint16_t jointCount;
		Entity *target;
		HumanBoneID targetJoint;
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

		void SetIKTarget(IKTarget::HumanBoneID tipJoint, uint16_t jointCount, Entity * target, IKTarget::HumanBoneID targetJoint = IKTarget::Root);

	private:
		Entity					entity;
		Model*					model;

		// constant buffer for bone matrices
		BufferHandle			boneMatricesBuffer;
		uint32_t				boneMatricesBufferSize;

		std::vector<AnimationLayer> layers;

		std::vector<IKTarget> iks;

	private:
		// update play back time and cross fade parameters
		void UpdateTiming();

		// calculate bone matrices and fill in the buffer
		int32_t FillInBoneMatrices(void* buffer, uint32_t bufferSize);

		void FABRIKSolve(uint16_t boneID, uint16_t limit, Entity * target, uint16_t targetJoint, math::float4x4 * matrices);
	
		void ReallocResources() {}
	};

	typedef Component<AnimationComponentData> AnimationComponent;
}
