#include "Transform.h"
#include "AnimationComponent.h"
#include "TofuMath.h"
#include <algorithm>
#include <set>
#include "TransformComponent.h"

using namespace std;
using namespace tofu::math;
using namespace tofu::model;

namespace tofu
{
	AnimationStateMachine * AnimationComponentData::GetStateMachine(size_t layer)
	{
		return &layers[layer].stateMachine;
	}

	int32_t AnimationComponentData::Play(string name, size_t layerIndex)
	{
		layers[layerIndex].stateMachine.Play(name);
		return kOK;
	}

	int32_t AnimationComponentData::CrossFade(string name, float duration, size_t layerIndex)
	{
		layers[layerIndex].stateMachine.CrossFade(name, duration);
		return kOK;
	}

	float AnimationComponentData::GetProgress(size_t layerIndex) const
	{
		return layers[layerIndex].stateMachine.GetPlaybackProgress();
	}

	void AnimationComponentData::SetIKTarget(IKTarget::HumanBoneID tipJoint, uint16_t jointCount, Entity* target, IKTarget::HumanBoneID targetJoint)
	{
		IKTarget ik { tipJoint, jointCount, target, targetJoint };
		iks.push_back(ik);
	}

	void AnimationComponentData::UpdateTiming()
	{
		for (AnimationLayer &layer : layers) {
			layer.Update(model);
		}
	}

	int32_t AnimationComponentData::FillInBoneMatrices(void* buffer, uint32_t bufferSize)
	{
		float4x4* matrices = reinterpret_cast<float4x4*>(buffer);

		if (nullptr == model)
		{
			return kErrUnknown;
		}

		if (static_cast<uint32_t>(sizeof(float4x4)) * model->header->NumBones > bufferSize)
		{
			return kErrUnknown;
		}

		// evaluate animation result
		EvaluateContext context(model);

		for (AnimationLayer &layer : layers) {
			layer.Evaluate(context);
		}

		// apply result
		for (auto i = 0; i < model->header->NumBones; i++) {
			matrices[i] = context.results[i].GetMatrix();

			// TODO: check available state before transition, then we can remove it
			// Set to T-pose if something wrong
			if (matrices[i] == glm::mat4()) {
				matrices[i] = model->bones[i].transformMatrix;
			}
		}

		// matrix to convert bone local space to model space 
		for (uint16_t i = 1; i < model->header->NumBones; i++)
		{
			uint16_t p = model->bones[i].parent;
			matrices[i] = matrices[p] * matrices[i];
		}

		for (IKTarget ik : iks) {
			FABRIKSolve(ik.tipJoint, ik.jointCount, ik.target, ik.targetJoint, matrices);
		}

		// append the offset matrices ( convert vertices from model space to bone local space )
		for (uint16_t i = 0; i < model->header->NumBones; i++)
		{
#ifdef TOFU_USE_GLM
			matrices[i] = transpose(matrices[i] * model->bones[i].offsetMatrix);
#else
			matrices[i] = matrices[i] * model->bones[i].offsetMatrix;
#endif
		}

		return kOK;
	}

	struct FABRIKChain {
		uint16_t boneIndex;

		// distance to its child
		float length;

		Transform transform;

		FABRIKChain(uint16_t boneIndex, float length, Transform transform)
			:boneIndex(boneIndex), length(length), transform(transform) {}
	};

	// FABRIK solver
	// http://www.andreasaristidou.com/FABRIK.html
	void AnimationComponentData::FABRIKSolve(uint16_t tipJointIndex, uint16_t jointCount, Entity* target, uint16_t targetJoint, float4x4* matrices)
	{
		// minimum difference between the effector and IK target
		static float minDifference = 1.0f;
		static int maxIteration = 10;

		if (tipJointIndex >= model->header->NumBones)
			return;

		ModelBone *bone = &model->bones[tipJointIndex];

		// no any parent bone to IK
		if (bone->parent == UINT16_MAX)
			return;

		// TODO: integrate with allocator
		Transform *transforms = static_cast<Transform*>(malloc(model->header->NumBones * sizeof(Transform)));

		for (uint16_t i = 0; i < model->header->NumBones; i++)
		{
			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;

			glm::decompose(matrices[i], scale, rotation, translation, skew, perspective);

			transforms[i] = { translation, glm::conjugate(rotation), scale };
		}

		vector<uint16_t> bones;
		bones.push_back(bone->id);

		// get all adjust joints
		for (uint16_t i = 0; i < jointCount; i++) {
			if (bone->parent == UINT16_MAX)
				break;

			bone = &model->bones[bone->parent];
			bones.push_back(bone->id);
		}

		std::reverse(bones.begin(), bones.end());

		std::vector<FABRIKChain> chains;
		chains.reserve(bones.size());

		float totalLength = 0;

		for (uint16_t i = 0; i < bones.size(); i++) {
			float length = 0;
			
			if (i < bones.size() - 1) {
				length = glm::length(transforms[bones[i]].GetTranslation() - transforms[bones[i + 1]].GetTranslation());
				totalLength += length;
			}

			chains.emplace_back(bones[i], length, transforms[bones[i]]);
		}
		
		// TODO: allow to set IK on target's joint
		if (targetJoint != IKTarget::Root)
			return;

		// convert to local position
		float3 targetPosition = entity.GetComponent<TransformComponent>()->WorldToLocalPosition(target->GetComponent<TransformComponent>()->GetWorldPosition());
		
		// target is out of reach
		if (totalLength < glm::length(targetPosition - transforms[bones[0]].GetTranslation())) {
			// forward
			for (int i = 0; i < bones.size() - 1; i++) {
				float l = chains[i].length / glm::length(targetPosition - chains[i].transform.GetTranslation());
				chains[i+1].transform.SetTranslation((1 - l) * chains[i].transform.GetTranslation() + l * targetPosition);
			}
		}
		else {
			size_t const tipBoneIndex = chains.size() - 1;

			// Check distance between tip location and effector location
			float difference = glm::length(targetPosition - chains[tipBoneIndex].transform.GetTranslation());
	
			size_t IterationCount = 0;
			while (difference > minDifference && IterationCount++ < maxIteration)
			{
				// set tip bone at end effector location.
				chains[tipBoneIndex].transform.SetTranslation(targetPosition);

				// backward reaching stage - adjust bones from end effector.
				for (size_t i = tipBoneIndex - 1; i > 0; i--)
				{
					float l = chains[i].length / glm::length(chains[i+1].transform.GetTranslation() - chains[i].transform.GetTranslation());
					chains[i].transform.SetTranslation((1 - l) * chains[i + 1].transform.GetTranslation() + l * chains[i].transform.GetTranslation());
				}

				// forward reaching stage - adjust bones from root.
				for (size_t i = 1; i <= tipBoneIndex; i++)
				{
					float l = chains[i - 1].length / glm::length(chains[i - 1].transform.GetTranslation() - chains[i].transform.GetTranslation());
					chains[i].transform.SetTranslation((1 - l) * chains[i - 1].transform.GetTranslation() + l * chains[i].transform.GetTranslation());
				}

				difference = glm::length(targetPosition - chains[tipBoneIndex].transform.GetTranslation());
			}
		}

		set<uint16_t> affectedBones;

		for (auto boneId : bones) {
			affectedBones.insert(tipJointIndex);
		}

		for (int i = bones.back(); i < model->header->NumBones; i++) {
			if (affectedBones.find(model->bones[i].parent) != affectedBones.end()) {
				affectedBones.insert(i);
			}
		}

		for (auto boneId : bones) {
			affectedBones.erase(tipJointIndex);
		}

		Transform *locals = static_cast<Transform*>(malloc(affectedBones.size() * sizeof(Transform)));
		int localIndex = 0;

		// iterate set in ascending order
		for (uint16_t id : affectedBones)
		{
			locals[localIndex] = transforms[id];
			locals[localIndex].SetToRelativeTransform(transforms[model->bones[id].parent]);
			localIndex++;
		}

		for (int i = 0; i < chains.size(); i++) {
			
			if (i < bones.size() - 1) {
				float3 oldDir = normalize(transforms[chains[i + 1].boneIndex].GetTranslation() - transforms[chains[i].boneIndex].GetTranslation());
				float3 newDir = normalize(chains[i + 1].transform.GetTranslation() - chains[i].transform.GetTranslation());

				// calculate axis of rotation from pre-translation vector to post-translation vector
				float3 rotationAxis = normalize(cross(oldDir, newDir));
				float rotationAngle = acos(dot(oldDir, newDir));
				quat deltaRotation = angleAxis(rotationAngle, rotationAxis);

				// calculate absolute rotation
				chains[i].transform.SetRotation(deltaRotation * chains[i].transform.GetRotation());
			}

			// override the original transform
			transforms[chains[i].boneIndex] = chains[i].transform;
		}

		localIndex = 0;

		// convert affected local to model
		for (uint16_t id : affectedBones)
		{
			transforms[id] = locals[localIndex++] * transforms[model->bones[id].parent];
		}

		for (uint16_t i = 0; i < model->header->NumBones; i++)
		{
			matrices[i] = transforms[i].GetMatrix();
		}

		free(transforms);
		free(locals);
	}
}
