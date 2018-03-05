#pragma once

#include "Common.h"

namespace tofu
{
	HANDLE_DECL(Material);

	enum MaterialType
	{
		kMaterialTypeTest,
		kMaterialTypeSkybox,
		kMaterialTypeOpaque,
		kMaterialTypeOpaqueSkinned,
		kMaterialTypeDepth,
		kMaterialTypeDepthSkinned,
		kMaterialDeferredGeometryOpaque,
		kMaterialDeferredGeometryOpaqueSkinned,
		kMaterialDeferredLightingOcclude,
		kMaterialDeferredLightingPointLight,
		kMaterialDeferredLightingSpotLight,
		kMaterialDeferredLightingAmbient,
		kMaxMaterialTypes
	};

	class Material
	{
		friend class RenderingSystem;
	public:

		void SetTexture(TextureHandle t) { mainTex = t; }

		void SetNormalMap(TextureHandle t) { normalMap = t; }
		void SetMetallicMap(TextureHandle t) { metallicMap = t; }

		void SetRoughnessMap(TextureHandle t) { roughnessMap = t; }

		void SetAoMap(TextureHandle t) { aoMap = t; }
		void SetSkyboxDiff(TextureHandle t) { skyboxDiffMap = t; }

		void SetSkyboxSpecMap(TextureHandle t) { skyboxSpecMap = t; }

		void SetLUTMap(TextureHandle t) { lutMap = t; }

	private:
		Material(MaterialType type = MaterialType::kMaterialTypeTest) 
			: 
			type(type),
			handle(),
			mainTex(),
			normalMap(),
			metallicMap(),
			roughnessMap(),
			aoMap()
		{}

	private:
		MaterialType	type;
		MaterialHandle	handle;
		TextureHandle	mainTex;
		TextureHandle	normalMap;
		TextureHandle   metallicMap;
		TextureHandle   roughnessMap;
		TextureHandle   aoMap;
		TextureHandle   skyboxDiffMap;
		TextureHandle   skyboxSpecMap;
		TextureHandle   lutMap;
	};
}