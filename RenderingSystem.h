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

		int32_t BeginFrame();

		int32_t Update() override;

		int32_t EndFrame();

		Model* CreateModel(const char* filename);

		TextureHandle CreateTexture(const char* filename);

		Material* CreateMaterial(MaterialType type);

	private:

		int32_t InitBuiltinShader(MaterialType matType, const char* vsFile, const char* psFile);

		int32_t ReallocAnimationResources(AnimationComponentData& c);

	private:
		Renderer*	renderer;

		HandleAllocator<ModelHandle, MAX_MODELS>			modelHandleAlloc;
		HandleAllocator<MeshHandle, MAX_MESHES>				meshHandleAlloc;
		HandleAllocator<MaterialHandle, MAX_MATERIALS>		materialHandleAlloc;

		std::unordered_map<std::string, ModelHandle>		modelTable;

		HandleAllocator<BufferHandle, MAX_BUFFERS>					bufferHandleAlloc;
		HandleAllocator<TextureHandle, MAX_TEXTURES>				textureHandleAlloc;
		HandleAllocator<SamplerHandle, MAX_SAMPLERS>				samplerHandleAlloc;
		HandleAllocator<VertexShaderHandle, MAX_VERTEX_SHADERS>		vertexShaderHandleAlloc;
		HandleAllocator<PixelShaderHandle, MAX_PIXEL_SHADERS>		pixelShaderHandleAlloc;
		HandleAllocator<PipelineStateHandle, MAX_PIPELINE_STATES>	pipelineStateHandleAlloc;

		size_t					frameNo;
		uint32_t				allocNo;

		BufferHandle			transformBuffer;
		uint32_t				transformBufferSize;

		BufferHandle			frameConstantBuffer;

		Mesh					meshes[MAX_MESHES];
		Model					models[MAX_MODELS];
		Material				materials[MAX_MATERIALS];

		PipelineStateHandle		materialPSOs[MaxMaterialTypes];
		VertexShaderHandle		materialVSs[MaxMaterialTypes];
		PixelShaderHandle		materialPSs[MaxMaterialTypes];
		SamplerHandle			defaultSampler;

		Model*					builtinCube;

		RendererCommandBuffer*	cmdBuf;
	};

}

