#pragma once

#include "Common.h"

#include "Entity.h"
#include "TransformComponent.h"
#include "ResourceManager.h"
#include <vector>
#include "../code_blank/GameplayDS.h"

namespace tofu
{
	class SceneManager
	{
	public:

		int32_t Init();

		int32_t LoadScene(const char* filename);

		std::vector<PathNode*>* GetPathNodes();

		int32_t GetPathNodesLength();

		tofu::math::float3 GetPlayerSpawnPoint();

	private:

		std::vector<PathNode*>* pathNodes;

		tofu::math::float3 pSpawnPoint;

		int32_t AddPathNode(const rapidjson::Value& value, int32_t);

		int32_t LoadSceneEntity(const rapidjson::Value& value, TransformComponent parent);

		int32_t LoadComponents(const rapidjson::Value& value, Entity e);

	private:
		ResourceManager		resMgr;
	};
}