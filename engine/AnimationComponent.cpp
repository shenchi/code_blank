#include "Transform.h"
#include "AnimationComponent.h"
#include "Engine.h"
#include "TofuMath.h"
#include "RenderingComponent.h"
#include <algorithm>
#include <cassert>

using namespace tofu::model;

namespace tofu
{
	AnimationStateMachine * AnimationComponentData::GetStateMachine(size_t layer)
	{
		return &layers[layer].stateMachine;
	}

	// FIXME: id
	int32_t AnimationComponentData::Play(uint32_t animId, size_t layerIndex)
	{
		if (animId == 0)
			layers[layerIndex].stateMachine.Play("idle");
		else
			layers[layerIndex].stateMachine.Play("walk");

		return kOK;
	}

	// FIXME: id
	int32_t AnimationComponentData::CrossFade(uint32_t animId, float duration, size_t layerIndex)
	{
		if (animId == 0)
			layers[layerIndex].stateMachine.CrossFade("idle", duration);
		else if(animId == 1)
			layers[layerIndex].stateMachine.CrossFade("walk", duration);
		else if (animId == 2)
			layers[layerIndex].stateMachine.CrossFade("run", duration);
		else
			layers[layerIndex].stateMachine.CrossFade("jump", duration);

		return kOK;
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

		// convert to world space 
		for (uint16_t i = 1; i < model->header->NumBones; i++)
		{
			uint16_t p = model->bones[i].parent;
			matrices[i] = matrices[p] * matrices[i];
		}

		// append the offset matrices ( convert vertices from model space to bone local space )
		for (uint16_t i = 0; i < model->header->NumBones; i++)
		{
			float* p = reinterpret_cast<float*>(&(model->bones[i].offsetMatrix));

#ifdef TOFU_USE_GLM
			matrices[i] = math::transpose(matrices[i] * model->bones[i].offsetMatrix);
#else
			matrices[i] = matrices[i] * model->bones[i].offsetMatrix;
#endif
		}

		return kOK;
	}
}
