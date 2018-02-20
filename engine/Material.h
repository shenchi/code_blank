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
		kMaxMaterialTypes
	};

	class Material
	{
		friend class RenderingSystem;
	public:

		void SetTexture(TextureHandle t) { mainTex = t; }

		void SetNormalMap(TextureHandle t) { normalMap = t; }

	private:
		Material(MaterialType type = MaterialType::kMaterialTypeTest) 
			: 
			type(type),
			handle(),
			mainTex(),
			normalMap()
		{}

	private:
		MaterialType	type;
		MaterialHandle	handle;
		TextureHandle	mainTex;
		TextureHandle	normalMap;
	};
}