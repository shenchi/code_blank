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
		friend class AnimationState;
		friend class AnimationStateMachine;
		friend class AnimationStateCache;
		friend struct EvaluateContext;

	private:
		ModelHandle					handle;
		MeshHandle					meshes[kMaxMeshesPerModel];
		uint32_t					numMeshes;
		uint32_t					vertexSize;
		model::ModelHeader*			header;
		model::ModelBone*			bones;
		model::ModelAnimation*		animations;
		model::AnimationTable		animationTable;
		model::ModelAnimChannel*	channels;
		model::ModelFloat3Frame*	translationFrames;
		model::ModelQuatFrame*		rotationFrames;
		model::ModelFloat3Frame*	scaleFrames;
		model::ModelAnimFrame*		frames;
		void*						rawData;
		size_t						rawDataSize;

	public:
		Model() :animationTable(model::AnimationTable()) {}
		TF_INLINE bool HasAnimation() const { return header->HasAnimation; }

		TF_INLINE model::ModelAnimation* GetAnimation(std::string name) { return &animations[animationTable[name]]; }
	};
}