#pragma once

#include "Common.h"

namespace tofu
{
	enum MaterialType
	{
		kMaterialNone,
		kMaterialSkybox,
		kMaterialShadowInstanced,
		kMaterialShadow,
		kMaterialShadowSkinned,
		kMaterialDeferredGeometryOpaqueInstanced,
		kMaterialDeferredGeometryOpaque,
		kMaterialDeferredGeometryOpaqueSkinned,
		kMaterialDeferredLightingOcclude,
		kMaterialDeferredLightingPointLight,
		kMaterialDeferredLightingSpotLight,
		kMaterialDeferredLightingAmbient,
		kMaterialDeferredTransparentInstanced,
		kMaterialDeferredTransparent,
		kMaterialDeferredTransparentSkinned,
		kMaterialPostProcessToneMapping,
		kMaterialPostProcessExtractBright,
		kMaterialPostProcessBlur,
		kMaterialPostProcessGaussianBlurH,
		kMaterialPostProcessGaussianBlurV,
		kMaterialPostProcessBloomApply,
		kMaterialPostProcessVolumetricFog,
		kMaterialPostProcessAntiAliasing,
		kMaterialOverlay,
		kMaterialFontRendering,
		kMaxMaterialTypes
	};

	struct MaterialParams
	{
		math::float4			color;
		math::float4			emissionColor;
		math::float4			texcoordParams;

		MaterialParams()
			:
			color{ 1, 1, 1, 1 },
			emissionColor{ 0, 0, 0, 0 },
			texcoordParams{ 1, 1, 0, 0 }
		{}
	};

	class Material
	{
		friend class RenderingSystem;
	public:

		void SetTexture(TextureHandle t) { mainTex = t; }

		void SetNormalMap(TextureHandle t) { normalMap = t; }

		void SetMetallicGlossMap(TextureHandle t) { metallicGlossMap = t; }

		void SetOcclusionMap(TextureHandle t) { occlusionMap = t; }

		void SetEmissionMap(TextureHandle t) { emissionMap = t; }

		void SetColor(const math::float4& color) { materialParams.color = color; isDirty = true; }

		void SetEmissionColor(const math::float4& color) { materialParams.emissionColor = color; isDirty = true; }

		void SetTextureParams(const math::float4& params) { materialParams.texcoordParams = params; isDirty = true; }

	private:
		Material(MaterialType type = MaterialType::kMaterialNone)
			: 
			type(type),
			handle(),
			mainTex(),
			normalMap(),
			metallicGlossMap(),
			occlusionMap(),
			emissionMap(),
			materialParamsBuffer(),
			materialParams(),
			isDirty(true)
		{}

	private:
		MaterialType	type;
		MaterialHandle	handle;
		TextureHandle	mainTex;
		TextureHandle	normalMap;
		TextureHandle	metallicGlossMap;
		TextureHandle	occlusionMap;
		TextureHandle	emissionMap;

		BufferHandle	materialParamsBuffer;
		MaterialParams	materialParams;
		bool			isDirty;
	};
}