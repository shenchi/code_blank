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
		kMaterialDeferredGeometryCutoffInstanced,
		kMaterialDeferredGeometryOpaque,
		kMaterialDeferredGeometryOpaqueSkinned,
		kMaterialDeferredGeometryCutoff,
		kMaterialDeferredGeometryCutoffSkinned,
		kMaterialDeferredLightingOcclude,
		kMaterialDeferredLightingPointLight,
		kMaterialDeferredLightingSpotLight,
		kMaterialDeferredLightingAmbient,
		kMaterialDeferredTransparentInstanced,
		kMaterialDeferredAdditiveInstanced,
		kMaterialDeferredTransparent,
		kMaterialDeferredTransparentSkinned,
		kMaterialDeferredAdditive,
		kMaterialDeferredAdditiveSkinned,
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
		math::float4			cutoff;

		MaterialParams()
			:
			color{ 1, 1, 1, 1 },
			emissionColor{ 0, 0, 0, 0 },
			texcoordParams{ 1, 1, 0, 0 },
			cutoff()
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

		bool HasEmissionColor() const { 
			const math::float4& c = materialParams.emissionColor;
			return c.x + c.y + c.z > 0.0f;
		}

		void SetCutoff(float cutoff) { materialParams.cutoff.x = cutoff; isDirty = true; }

		void SetAdditiveFactor(float cutoff) { materialParams.cutoff.y = cutoff; isDirty = true; }

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