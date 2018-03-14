#pragma once

#include "Common.h"

namespace tofu
{
	enum MaterialType
	{
		kMaterialTypeSkybox,
		kMaterialTypeOpaque,
		kMaterialTypeOpaqueSkinned,
		kMaterialTypeDepth,
		kMaterialTypeDepthSkinned,
		kMaterialShadow,
		kMaterialShadowSkinned,
		kMaterialDeferredGeometryOpaque,
		kMaterialDeferredGeometryOpaqueSkinned,
		kMaterialDeferredLightingOcclude,
		kMaterialDeferredLightingPointLight,
		kMaterialDeferredLightingSpotLight,
		kMaterialDeferredLightingAmbient,
		kMaterialDeferredTransparent,
		kMaterialDeferredTransparentSkinned,
		kMaterialPostProcessToneMapping,
		kMaterialPostProcessExtractBright,
		kMaterialPostProcessBlur,
		kMaterialPostProcessVolumetricFog,
		kMaxMaterialTypes
	};

	class Material
	{
		friend class RenderingSystem;
	public:

		void SetTexture(TextureHandle t) { mainTex = t; }

		void SetNormalMap(TextureHandle t) { normalMap = t; }

		void SetMetallicGlossMap(TextureHandle t) { metallicGlossMap = t; }

		void SetOcclusionMap(TextureHandle t) { occlusionMap = t; }


		void SetMetallicMap(TextureHandle t) { metallicMap = t; }

		void SetRoughnessMap(TextureHandle t) { roughnessMap = t; }

		void SetAoMap(TextureHandle t) { aoMap = t; }
		void SetSkyboxDiff(TextureHandle t) { skyboxDiffMap = t; }

		void SetSkyboxSpecMap(TextureHandle t) { skyboxSpecMap = t; }

		void SetLUTMap(TextureHandle t) { lutMap = t; }

	private:
		Material(MaterialType type = MaterialType::kMaterialTypeSkybox) 
			: 
			type(type),
			handle(),
			mainTex(),
			normalMap(),
			metallicGlossMap(),
			occlusionMap(),
			metallicMap(),
			roughnessMap(),
			aoMap()
		{}

	private:
		MaterialType	type;
		MaterialHandle	handle;
		TextureHandle	mainTex;
		TextureHandle	normalMap;
		TextureHandle	metallicGlossMap;
		TextureHandle	occlusionMap;

		TextureHandle   metallicMap;
		TextureHandle   roughnessMap;
		TextureHandle   aoMap;
		TextureHandle   skyboxDiffMap;
		TextureHandle   skyboxSpecMap;
		TextureHandle   lutMap;
	};
}