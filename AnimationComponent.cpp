#include "AnimationComponent.h"

#include "TofuMath.h"
#include "RenderingComponent.h"

namespace tofu
{

	int32_t AnimationComponentData::FillInBoneMatrices(void* buffer, uint32_t bufferSize, uint32_t animId, float time)
	{
		math::float4x4* matrices = reinterpret_cast<math::float4x4*>(buffer);

		RenderingComponent renderable = entity.GetComponent<RenderingComponent>();
		
		if (model != renderable->GetModel())
		{
			model = renderable->GetModel();
		}

		if (nullptr == model || !model->HasAnimation())
		{
			return TF_UNKNOWN_ERR;
		}

		
		return TF_OK;
	}

}
