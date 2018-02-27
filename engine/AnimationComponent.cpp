#include "Transform.h"
#include "AnimationComponent.h"
#include "Engine.h"
#include "TofuMath.h"
#include "RenderingComponent.h"
#include <algorithm>
#include <cassert>

#include <glm/gtx/matrix_decompose.hpp>
#include "TransformComponent.h"
#include <set>

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

	bool AnimationComponentData::SetIKPosition(AvatarIKGoal goal, float3 position)
	{
		return false;
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

		// load bone matrices
		for (uint16_t i = 0; i < model->header->NumBones; i++)
		{
			matrices[i] = model->bones[i].transform;
		}

		// evaluate animation result
		EvaluateContext context(model);

		for (AnimationLayer &layer : layers) {
			layer.Evaluate(context);
		}

		// apply result
		for (auto i = 0; i < model->header->NumBones; i++) {
			if (context.transforms[i].isDirty) {
				matrices[i] = context.transforms[i].GetMatrix();
			}
		}

		// matrix to convert bone local space to model space 
		for (uint16_t i = 1; i < model->header->NumBones; i++)
		{
			uint16_t p = model->bones[i].parent;
			matrices[i] = matrices[p] * matrices[i];
		}

		// FIXME: IK test function
		FABRIKSolve(11, 2, float3(500, 100, 500), matrices);
		FABRIKSolve(58, 2, float3(500, 100, 500), matrices);

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
	void AnimationComponentData::FABRIKSolve(uint16_t boneID, uint16_t limit, float3 target, float4x4* matrices)
	{
		// minimum difference between the effector and IK target
		static float minDifference = 1.0f;
		static int maxIteration = 10;

		ModelBone *bone = &model->bones[boneID];

		// no any parent bone to IK
		if (bone->parent == UINT16_MAX)
			return;

		vector<Transform> transforms;
		transforms.reserve(model->header->NumBones);

		for (uint16_t i = 0; i < model->header->NumBones; i++)
		{
			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;

			glm::decompose(matrices[i], scale, rotation, translation, skew, perspective);

			rotation = glm::conjugate(rotation);

			transforms.emplace_back(translation, rotation, scale);
		}

		vector<uint16_t> bones;
		bones.push_back(bone->id);

		for (uint16_t i = 0; i < limit; i++) {
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

		// convert to local position
		target = entity.GetComponent<TransformComponent>()->WorldToLocalPosition(target);

		// FIXME: for test
		target = transforms[boneID].GetTranslation() + float3(0.f, 20.0f, 0.f);

		// target is out of reach
		if (totalLength < glm::length(target - transforms[bones.back()].GetTranslation())) {
			// forward
			for (int i = 0; i < bones.size() - 1; i++) {
				float l = chains[i].length / glm::length(target - chains[i].transform.GetTranslation());
				chains[i+1].transform.SetTranslation((1 - l) * chains[i].transform.GetTranslation() + l * target);
			}
		}
		else {
			size_t const tipBoneIndex = chains.size() - 1;

			// Check distance between tip location and effector location
			float difference = glm::length(target - chains[tipBoneIndex].transform.GetTranslation());
	
			size_t IterationCount = 0;
			while ((difference > minDifference) && (IterationCount++ < maxIteration))
			{
				// set tip bone at end effector location.
				chains[tipBoneIndex].transform.SetTranslation(target);

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

				difference = glm::length(target - chains[tipBoneIndex].transform.GetTranslation());
			}
		}

		set<uint16_t> effectBones;

		for (auto boneId : bones) {
			effectBones.insert(boneID);
		}

		for (int i = bones.back(); i < model->header->NumBones; i++) {
			if (effectBones.find(model->bones[i].parent) != effectBones.end()) {
				effectBones.insert(i);
			}
		}

		for (auto boneId : bones) {
			effectBones.erase(boneID);
		}

		vector<Transform> locals;
		locals.reserve(effectBones.size());

		// TODO: make sure set is order ascending
		for (uint16_t id : effectBones)
		{
			locals.push_back(transforms[id]);
			locals.back().SetToRelativeTransform(transforms[model->bones[id].parent]);
		}

		for (int i = 0; i < chains.size(); i++) {
			
			if (i < bones.size() - 1) {
				float3 oldDir = normalize(transforms[chains[i + 1].boneIndex].GetTranslation() - transforms[chains[i].boneIndex].GetTranslation());
				float3 newDir = normalize(chains[i + 1].transform.GetTranslation() - chains[i].transform.GetTranslation());

				// calculate axis of rotation from pre-translation vector to post-translation vector
				float3 rotationAxis = normalize(cross(oldDir, newDir));
				float rotationAngle = acos(dot(oldDir, newDir));
				math::quat deltaRotation = angleAxis(rotationAngle, rotationAxis);

				// calculate absolute rotation
				chains[i].transform.SetRotation(deltaRotation * chains[i].transform.GetRotation());

				// FIXME: need normalize?
				//chains[i].transform.SetRotation(normalize(deltaRotation * chains[i].transform.GetRotation()));
			}

			// override the original transform
			transforms[chains[i].boneIndex] = chains[i].transform;
		}

		int localIndex = 0;

		for (uint16_t id : effectBones)
		{
			transforms[id] = locals[localIndex++] * transforms[model->bones[id].parent];
		}

		for (uint16_t i = 0; i < model->header->NumBones; i++)
		{
			matrices[i] = transforms[i].GetMatrix();
		}
	}
}
