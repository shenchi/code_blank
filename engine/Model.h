#pragma once

#include "Common.h"
#include "ModelFormat.h"

namespace tofu
{
	class RenderingSystem;

	HANDLE_DECL(Mesh);
	HANDLE_DECL(Model);

	class Model
	{
		friend class RenderingSystem;
		friend class AnimationComponentData;
	public:

		TF_INLINE bool HasAnimation() const { return header->HasAnimation; }

	private:
		ModelHandle					handle;
		MeshHandle					meshes[kMaxMeshesPerModel];
		uint32_t					numMeshes;
		uint32_t					vertexSize;
		model::ModelHeader*			header;
		model::ModelBone*			bones;
		model::ModelAnimation*		animations;
		model::ModelAnimChannel*	channels;
		model::ModelFloat3Frame*	translationFrames;
		model::ModelQuatFrame*		rotationFrames;
		model::ModelFloat3Frame*	scaleFrames;
		void*						rawData;
		size_t						rawDataSize;
	};
}