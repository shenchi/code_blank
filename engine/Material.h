#pragma once

#include "Common.h"

namespace tofu
{
	HANDLE_DECL(Material);

	enum MaterialType
	{
		TestMaterial,
		SkyboxMaterial,
		OpaqueMaterial,
		OpaqueSkinnedMaterial,
		MaxMaterialTypes
	};

	class Material
	{
		friend class RenderingSystem;
	public:

		void SetTexture(TextureHandle t) { mainTex = t; }

		void SetNormalMap(TextureHandle t) { normalMap = t; }

	private:
		Material(MaterialType type = MaterialType::TestMaterial) 
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