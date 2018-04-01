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

		int32_t SwapBuffers();

		Model* CreateModel(const char* filename, uint32_t label = kResourceLevel);

		TextureHandle CreateTexture(const char* filename, uint32_t label = kResourceLevel);

		TextureHandle CreateTexture(PixelFormat format, uint32_t width, uint32_t height, uint32_t arraySize = 1, uint32_t pitch = 0, void* data = nullptr, uint32_t binding = kBindingShaderResource, uint32_t label = kResourceLevel);

		TextureHandle CreateTexture(TextureHandle source, uint32_t startIndex, uint32_t arraySize = 1, uint32_t binding = kBindingShaderResource, PixelFormat format = kFormatAuto, uint32_t label = kResourceLevel);

		TextureHandle CreateTexture(PixelFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t pitch = 0, uint32_t slicePitch = 0, void* data = nullptr, uint32_t binding = kBindingShaderResource, uint32_t label = kResourceLevel);

		Material* CreateMaterial(MaterialType type, uint32_t label = kResourceLevel);

		int32_t CleanupLevelResources();

	private:

		BufferHandle CreateConstantBuffer(uint32_t size, bool dynamic = true, uint32_t label = kResourceGlobal);

		int32_t UpdateConstantBuffer(BufferHandle buffer, uint32_t size, void* data);

		int32_t InitBuiltinMaterial(MaterialType matType, const char* vsFile, const char* psFile);

		int32_t LoadVertexShader(const char* filename, VertexShaderHandle& handle, uint32_t label = kResourceGlobal);

		int32_t LoadPixelShader(const char* filename, PixelShaderHandle& handle, uint32_t label = kResourceGlobal);

		int32_t LoadComputeShader(const char* filename, ComputeShaderHandle& handle, uint32_t label = kResourceGlobal);

		int32_t ReallocAnimationResources(AnimationComponentData& c);

		int32_t DeferredPipeline();

		TF_INLINE void AddLevelResourceHandle(BaseHandle handle)
		{ 
			assert(numLevelResources < kMaxLevelResources);
			assert(handle);
			levelResources[numLevelResources++] = handle; 
		}

	private:

		HandleAllocator<ModelHandle, kMaxModels>			modelHandleAlloc;
		HandleAllocator<MeshHandle, kMaxMeshes>				meshHandleAlloc;
		HandleAllocator<MaterialHandle, kMaxMaterials>		materialHandleAlloc;

		HandleAllocator<BufferHandle, kMaxBuffers>					bufferHandleAlloc;
		HandleAllocator<TextureHandle, kMaxTextures>				textureHandleAlloc;
		HandleAllocator<SamplerHandle, kMaxSamplers>				samplerHandleAlloc;
		HandleAllocator<VertexShaderHandle, kMaxVertexShaders>		vertexShaderHandleAlloc;
		HandleAllocator<PixelShaderHandle, kMaxPixelShaders>		pixelShaderHandleAlloc;
		HandleAllocator<ComputeShaderHandle, kMaxComputeShaders>	computeShaderHandleAlloc;
		HandleAllocator<PipelineStateHandle, kMaxPipelineStates>	pipelineStateHandleAlloc;

		BaseHandle				levelResources[kMaxLevelResources];
		uint32_t				numLevelResources;

		std::unordered_map<std::string, ModelHandle>		modelTable;

		Renderer*				renderer;

		size_t					frameNo;

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
		SamplerHandle           shadowSampler;
		SamplerHandle           lutSampler;

		Model*					builtinQuad;
		Model*					builtinCube;
		Model*					builtinSphere;
		Model*					builtinCone;

		TextureHandle			defaultAlbedoMap;
		TextureHandle			defaultNormalMap;
		TextureHandle			defaultMetallicGlossMap;
		TextureHandle			defaultOcclusionMap;
		TextureHandle			defaultEmissionMap;
		TextureHandle			lutMap;

		RendererCommandBuffer*	cmdBuf;

		// resources for deferred shading
		TextureHandle			gBuffer1;
		TextureHandle			gBuffer2;
		TextureHandle			gBuffer3;
		TextureHandle			gBuffer4;

		TextureHandle			hdrTarget;
		TextureHandle			hdrTarget2;

		BufferHandle			ambientDirLightBuffer;
		BufferHandle			pointLightBuffer;
		BufferHandle			spotLightBuffer;
		BufferHandle			pointLightTransformBuffer;
		BufferHandle			spotLightTransformBuffer;
		BufferHandle			shadowMatricesBuffer;

		TextureHandle			shadowMapArray;
		TextureHandle			shadowMaps[kMaxShadowCastingLights];

		// resources for volumetric fog
		ComputeShaderHandle		injectionShader;
		ComputeShaderHandle		scatterShader;

		TextureHandle			injectionTex;
		TextureHandle			scatterTex;

		BufferHandle			fogParamsBuffer;
	};

}

