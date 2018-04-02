#pragma once

#include "Common.h"
#include "ModelFormat.h"

namespace tofu
{
	class RenderingSystem;

	class Model
	{
		friend class RenderingSystem;
		friend class AnimationComponentData;
		friend class AnimationState;
		friend class AnimationStateMachine;
		friend class AnimationStateCache;
		friend struct EvaluateContext;
	
	public:
		TF_INLINE uint32_t GetStride() const { return vertexSize; }

		TF_INLINE uint32_t GetNumMeshes() const { return numMeshes; }

		TF_INLINE const float* GetVertices() const { return vertices; }

		TF_INLINE const float* GetVertices(uint32_t mesh) const 
		{ 
			return reinterpret_cast<float*>(
				reinterpret_cast<uint8_t*>(vertices) + baseVertex[mesh] * vertexSize
				);
		}

		TF_INLINE const uint16_t* GetIndices(uint32_t mesh) const { return indices[mesh]; }

		TF_INLINE uint32_t GetNumVertices() const { return numVertices; }

		TF_INLINE uint32_t GetNumIndices(uint32_t mesh) const { return numIndices[mesh]; }

	private:
		ModelHandle					handle;
		MeshHandle					meshes[kMaxMeshesPerModel];
		uint32_t					numVertices;
		uint32_t					numMeshes;
		uint32_t					vertexSize;
		model::ModelHeader*			header;
		model::ModelBone*			bones;
		model::ModelAnimation*		animations;
		model::AnimationTable		animationTable;
		model::ModelAnimFrame*		frames;
		float*						vertices;
		uint16_t*					indices[kMaxMeshesPerModel];
		uint32_t					baseVertex[kMaxMeshesPerModel];
		uint32_t					numIndices[kMaxMeshesPerModel];
		void*						rawData;
		size_t						rawDataSize;

	public:
		//Model() :animationTable(model::AnimationTable()) {}
		TF_INLINE bool HasAnimation() const { return header->HasAnimation; }

		TF_INLINE model::ModelAnimation* GetAnimation(std::string name) { 
			if (animationTable.find(name) == animationTable.end()) {
				return nullptr;
			}
			return &animations[animationTable[name]]; 
		}
	};
}