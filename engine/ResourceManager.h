#pragma once

#include "Common.h"
#include <rapidjson/document.h>
#include <unordered_map>
#include <string>

namespace tofu
{
	class Material;

	class ResourceManager
	{
		//SINGLETON_DECL(ResourceManager);

	public:

		int32_t Init();

		Material* LoadMaterial(const char* name);

		TextureHandle LoadTexture(const char* path);

		void SetDefaultAlbedoMap(TextureHandle tex) { defaultAlbedoMap = tex; }

		void SetDefaultNormalMap(TextureHandle tex) { defaultNormalMap = tex; }

	private:

		int32_t LoadConfig();

	private:
		rapidjson::Document		config;

		TextureHandle			defaultAlbedoMap;
		TextureHandle			defaultNormalMap;

		std::unordered_map<std::string, Material*>	materials;
		std::unordered_map<std::string, TextureHandle> textures;
		
		std::unordered_map<std::string, size_t>		matTable;
		std::unordered_map<std::string, size_t>		texTable;
	};
}