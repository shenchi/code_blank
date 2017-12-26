#pragma once

#include "Common.h"
#include "Module.h"
#include "Model.h"
#include "Material.h"

#include "Renderer.h"

#include "HandleAllocator.h"

#include <unordered_map>
#include <string>

namespace tofu
{
	class AnimationComponentData;

	struct Mesh
	{
		BufferHandle	VertexBuffer;
		BufferHandle	IndexBuffer;
		uint32_t		StartIndex;
		uint32_t		StartVertex;
		uint32_t		NumIndices;
		uint32_t		NumVertices;
	};

	class RenderingSystem : public Module
	{
		SINGLETON_DECL(RenderingSystem)

	public:
		RenderingSystem();
		~RenderingSystem();

	public:
		int32_t Init() override;

		int32_t Shutdown() override;

		// prepare for one frame, this function should be call before other module's update()
		int32_t BeginFrame();

		int32_t Update() override;

		// submit all render commands to backend
		int32_t EndFrame();

		Model* CreateModel(const char* filename);

		TextureHandle CreateTexture(const char* filename);

		Material* CreateMaterial(MaterialType type);

	private:

		int32_t InitBuiltinShader(MaterialType matType, const char* vsFile, const char* psFile);

		int32_t ReallocAnimationResources(AnimationComponentData& c);

	private:
		Renderer*	renderer;

		HandleAllocator<ModelHandle, kMaxModels>			modelHandleAlloc;
		HandleAllocator<MeshHandle, kMaxMeshes>				meshHandleAlloc;
		HandleAllocator<MaterialHandle, kMaxMaterials>		materialHandleAlloc;

		std::unordered_map<std::string, ModelHandle>		modelTable;

		HandleAllocator<BufferHandle, kMaxBuffers>					bufferHandleAlloc;
		HandleAllocator<TextureHandle, kMaxTextures>				textureHandleAlloc;
		HandleAllocator<SamplerHandle, kMaxSamplers>				samplerHandleAlloc;
		HandleAllocator<VertexShaderHandle, kMaxVertexShaders>		vertexShaderHandleAlloc;
		HandleAllocator<PixelShaderHandle, kMaxPixelShaders>		pixelShaderHandleAlloc;
		HandleAllocator<PipelineStateHandle, kMaxPipelineStates>	pipelineStateHandleAlloc;

		size_t					frameNo;
		uint32_t				allocNo;

		BufferHandle			transformBuffer;
		uint32_t				transformBufferSize;

		BufferHandle			frameConstantBuffer;

		Mesh					meshes[kMaxMeshes];
		Model					models[kMaxModels];
		Material				materials[kMaxMaterials];

		PipelineStateHandle		materialPSOs[kMaxMaterialTypes];
		VertexShaderHandle		materialVSs[kMaxMaterialTypes];
		PixelShaderHandle		materialPSs[kMaxMaterialTypes];
		SamplerHandle			defaultSampler;

		Model*					builtinCube;

		RendererCommandBuffer*	cmdBuf;
	};

}

