#pragma once

#include "Component.h"
#include "Renderer.h"

namespace tofu
{
	class RenderingSystem;
	class Model;
	class Material;

	class SkinnedMeshComponentData
	{
		friend class RenderingSystem;

	public:
		SkinnedMeshComponentData() : SkinnedMeshComponentData(Entity()) {}
		SkinnedMeshComponentData(Entity e)
			:
			entity(e),
			boneBuffer(),
			model(nullptr),
			material(nullptr)
		{}

		void SetModel(Model* model);

		void SetMaterial(Material* material);

	private:
		Entity				entity;
		BufferHandle		boneBuffer;
		Model*				model;
		Material*			material;

		int32_t				currentAnimation;
		float				currentTime;
	};

	typedef Component<SkinnedMeshComponentData> SkinnedMeshComponent;
}