#include "LightComponent.h"
#include "RenderingSystem.h"

namespace tofu {
	TextureHandle LightComponentData::CreateDepthMap()
	{
		uint32_t width = 1024, height = 1024;
		TextureHandle ret = RenderingSystem::instance()->CreateDepthMap(width, height);

		return ret;
	}
}