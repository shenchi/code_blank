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

		TF_INLINE uint32_t GetStride() const { return vertexSize; }

		TF_INLINE uint32_t GetNumMeshes() const { return numMeshes; }

		TF_INLINE const float* GetVertices(uint32_t mesh) const { return vertices[mesh]; }

		TF_INLINE const uint16_t* GetIndices(uint32_t mesh) const { return indices[mesh]; }

		TF_INLINE uint32_t GetNumVertices(uint32_t mesh) const { return numVertices[mesh]; }

		TF_INLINE uint32_t GetNumIndices(uint32_t mesh) const { return numIndices[mesh]; }

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
		model::ModelAnimFrame*		frames;
		float*						vertices[kMaxMeshesPerModel];
		uint16_t*					indices[kMaxMeshesPerModel];
		uint32_t					numVertices[kMaxMeshesPerModel];
		uint32_t					numIndices[kMaxMeshesPerModel];
		void*						rawData;
		size_t						rawDataSize;
	};
}