#pragma once

#include "Common.h"

#include "Entity.h"
#include "TransformComponent.h"
#include "ResourceManager.h"

namespace tofu
{
	class SceneManager
	{
	public:

		int32_t Init();

		int32_t LoadScene(const char* filename);

	private:

		int32_t LoadSceneEntity(const rapidjson::Value& value, TransformComponent parent);

		int32_t LoadComponents(const rapidjson::Value& value, Entity e);

	private:
		ResourceManager		resMgr;
	};
}