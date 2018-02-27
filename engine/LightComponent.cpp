#include "LightComponent.h"
#include "RenderingSystem.h"
#include "TransformComponent.h"

namespace tofu {

	using namespace math;

	void LightComponentData::CreateDepthMap()
	{
		uint32_t width = 1600, height = 900;
		TextureHandle ret = RenderingSystem::instance()->CreateDepthMap(width, height);
		depthMap = ret;
		castShadow = true;
	}
}