#include "LightComponent.h"
#include "RenderingSystem.h"
#include "TransformComponent.h"

namespace tofu {

	using namespace math;

	void LightComponentData::CreateDepthMap()
	{
		uint32_t width = 1024, height = 1024;
		TextureHandle ret = RenderingSystem::instance()->CreateDepthMap(width, height);
		shadowMat = RenderingSystem::instance()->CreateMaterial(kMaterialTypeDepth);
		shadowMat->SetTexture(ret);
		depthMap = ret;
	}
}