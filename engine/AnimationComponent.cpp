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
	int32_t AnimationComponentData::Play(uint32_t animId)
	{
		if (animId == 0)
			stateMachine.Play("idle");
		else
			stateMachine.Play("walk");

		return kOK;
	}

	int32_t AnimationComponentData::CrossFade(uint32_t animId, float duration)
	{
		if (animId == 0)
			stateMachine.CrossFade("idle", duration);
		else
			stateMachine.CrossFade("walk", duration);

		return kOK;
	}

	void AnimationComponentData::UpdateTiming()
	{
		// FIXME: Transition
		UpdateContext context{ model };
		stateMachine.Update(context);
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

		EvaluateContext context(model);
		stateMachine.Evaluate(context, 1.0f);

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
