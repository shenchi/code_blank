#include "Transform.h"
#include "AnimationComponent.h"
#include "Engine.h"
#include "TofuMath.h"
#include "RenderingComponent.h"
#include <algorithm>
#include <cassert>

#include <glm/gtx/matrix_decompose.hpp>
#include "TransformComponent.h"

using namespace tofu::model;

namespace tofu
{
	AnimationStateMachine * AnimationComponentData::GetStateMachine(size_t layer)
	{
		return &layers[layer].stateMachine;
	}

	int32_t AnimationComponentData::Play(std::string name, size_t layerIndex)
	{
		layers[layerIndex].stateMachine.Play(name);
		return kOK;
	}

	int32_t AnimationComponentData::CrossFade(std::string name, float duration, size_t layerIndex)
	{
		layers[layerIndex].stateMachine.CrossFade(name, duration);
		return kOK;
	}

	bool AnimationComponentData::SetIKPosition(AvatarIKGoal goal, math::float3 position)
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
		math::float4x4* matrices = reinterpret_cast<math::float4x4*>(buffer);

		if (nullptr == model)
		{
			return kErrUnknown;
		}

		if (static_cast<uint32_t>(sizeof(math::float4x4)) * model->header->NumBones > bufferSize)
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

		std::vector<Transform> transforms;
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

		Solve(10, math::float3(500, 100, 500), &transforms[0]);

		for (uint16_t i = 0; i < model->header->NumBones; i++)
		{
			matrices[i] = transforms[i].GetMatrix();
		}

		// append the offset matrices ( convert vertices from model space to bone local space )
		for (uint16_t i = 0; i < model->header->NumBones; i++)
		{
#ifdef TOFU_USE_GLM
			matrices[i] = math::transpose(matrices[i] * model->bones[i].offsetMatrix);
#else
			matrices[i] = matrices[i] * model->bones[i].offsetMatrix;
#endif
		}

		return kOK;
	}

	// backward reaching; set end effector as target
	void AnimationComponentData::Backward(uint16_t boneID, math::float3 target, Transform *transforms) {

		float lengths[2];

		ModelBone *bone = &model->bones[boneID];

		for (int i = 0; i < 2; i++) {
			lengths[i] = (transforms[boneID].GetTranslation() - transforms[bone->parent].GetTranslation()).length();
			bone = &model->bones[bone->parent];
		}

		transforms[boneID].SetTranslation(target);
		bone = &model->bones[boneID];

		for (int i = 0; i < 2; i++) {
			float l = lengths[i] / (transforms[boneID].GetTranslation() - transforms[bone->parent].GetTranslation()).length();
			math::float3 position = (1 - l) * transforms[boneID].GetTranslation() + l * transforms[bone->parent].GetTranslation();
			transforms[bone->parent].SetTranslation(position);
			bone = &model->bones[bone->parent];
		}
	}

	//// forward reaching; set root at initial position
	//void AnimationComponentData::Forward(uint16_t boneID, math::float3 target, Transform *transforms) {

	//	float lengths[2];

	//	ModelBone *bone = &model->bones[boneID];

	//	for (int i = 0; i < 2; i++) {
	//		lengths[i] = (transforms[boneID].GetTranslation() - transforms[bone->parent].GetTranslation()).length();
	//		bone = &model->bones[bone->parent];
	//	}

	//	transforms[boneID].SetTranslation(target);
	//	bone = &model->bones[boneID];

	//	for (int i = 0; i < 2; i++) {
	//		float l = lengths[i] / (transforms[boneID].GetTranslation() - transforms[bone->parent].GetTranslation()).length();
	//		math::float3 position = (1 - l) * transforms[boneID].GetTranslation() + l * transforms[bone->parent].GetTranslation();
	//		transforms[bone->parent].SetTranslation(position);
	//		bone = &model->bones[bone->parent];
	//	}
	//}

	void AnimationComponentData::Solve(uint16_t boneID, math::float3 target, Transform *transforms) 
	{
		ModelBone *bone = &model->bones[boneID];

		// no any parent bone to IK
		if (bone->parent == UINT16_MAX)
			return;

		static int limit = 2;

		std::vector<uint16_t> bones;
		std::vector<float> lengths;
		float totalLength = 0;

		bones.push_back(bone->id);

		for (int i = 0; i < limit; i++) {
			if (bone->parent == UINT16_MAX)
				break;

			bone = &model->bones[bone->parent];
			bones.push_back(bone->id);

			float length = glm::length(transforms[bones[i]].GetTranslation() - transforms[bones[i + 1]].GetTranslation());
			lengths.push_back(length);
			totalLength += length;
		}

		// convert to local position
		target = entity.GetComponent<TransformComponent>()->WorldToLocalPosition(target);

		// target is out of reach
		if (totalLength < glm::length(target - transforms[bones.back()].GetTranslation())) {

			// forward
			for (int i = bones.size() - 2; i >= 0; i--) {
				float l = lengths[i] / glm::length(target - transforms[bones[i + 1]].GetTranslation());
				math::float3 position = (1 - l) * transforms[bones[i + 1]].GetTranslation() + l * target;
				transforms[bones[i]].SetTranslation(position);
			}
		}
		else {
			// TODO:
		}
	}
		
	/*	
	function chain : forward()
		--forward reaching; set root at initial position
		self.joints[1] = self.origin.p;
	for i = 1, self.n - 1 do
		local r = (self.joints[i + 1] - self.joints[i]);
	local l = self.lengths[i] / r.magnitude;
	--find new joint position
		local pos = (1 - l) * self.joints[i] + l * self.joints[i + 1];
	self.joints[i + 1] = pos;
	end;
	end;

	function chain : solve()
		local distance = (self.joints[1] - self.target).magnitude;
	if distance > self.totallength then
		-- target is out of reach
		for i = 1, self.n - 1 do
			local r = (self.target - self.joints[i]).magnitude;
	local l = self.lengths[i] / r;
	--find new joint position
		self.joints[i + 1] = (1 - l) * self.joints[i] + l * self.target;
	end;
	else
		--target is in reach
		local bcount = 0;
	local dif = (self.joints[self.n] - self.target).magnitude;
	while dif > self.tolerance do --check if within error margin
		self : backward();
self:forward();
	dif = (self.joints[self.n] - self.target).magnitude;
	-- break if it's taking too long so the game doesn't freeze
		bcount = bcount + 1;
	if bcount > 10 then break; end;
	end;
	end;
	end;*/
}
