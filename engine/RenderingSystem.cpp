#include "RenderingSystem.h"

#include <cassert>

#include "Renderer.h"

#include "MemoryAllocator.h"
#include "FileIO.h"

#include "ModelFormat.h"

#include "TransformComponent.h"
#include "CameraComponent.h"
#include "RenderingComponent.h"
#include "AnimationComponent.h"
#include "LightComponent.h"

namespace
{
	struct FrameConstants							// 32 shader constants in total
	{
		tofu::math::float4x4	matView;			// 4 shader constants
		tofu::math::float4x4	matProj;			// 4 shader constants
		float					padding1[32];		// 8 shader constants
		tofu::math::float3		cameraPos;			// 1 shader constants
		float					padding2;
		float					padding3[4 * 15];	// 15 shader constants
	};

	struct LightingConstants {                      // 16 shader constants in total
		tofu::math::float4	    lightColor[256];			// 1 shader constants
		tofu::math::float4	    lightDirection[256];	    // float4 for alignment, actually it's float3, 1 shader constants
		float                   count;
		tofu::math::float3      camPos;
		tofu::math::float4      lightPosition[256];
		tofu::math::float4      type[256];    
		tofu::math::float4		_reserv1[3071];	
	};

}

namespace tofu
{
	SINGLETON_IMPL(RenderingSystem);

	RenderingSystem::RenderingSystem()
		:
		renderer(nullptr),
		modelHandleAlloc(),
		meshHandleAlloc(),
		materialHandleAlloc(),
		modelTable(),
		bufferHandleAlloc(),
		textureHandleAlloc(),
		samplerHandleAlloc(),
		vertexShaderHandleAlloc(),
		pixelShaderHandleAlloc(),
		pipelineStateHandleAlloc(),
		frameNo(0),
		allocNo(0),
		transformBuffer(),
		transformBufferSize(0),
		frameConstantBuffer(),
		lightingConstantBuffer(),
		meshes(),
		models(),
		materials(),
		materialPSOs(),
		materialVSs(),
		materialPSs(),
		defaultSampler(),
		shadowSampler(),
		builtinCube(),
		cmdBuf(nullptr)
	{
		assert(nullptr == _instance);
		_instance = this;

		renderer = Renderer::CreateRenderer();
	}

	RenderingSystem::~RenderingSystem()
	{
		delete renderer;
	}

	int32_t RenderingSystem::Init()
	{
		// Initialize Memory Management for Rendering
		CHECKED(MemoryAllocator::Allocators[kAllocLevelBasedMem].Init(
			kLevelBasedMemSize,
			kLevelBasedMemAlign));

		for (uint32_t i = kAllocFrameBasedMem;
			i <= kAllocFrameBasedMemEnd;
			++i)
		{
			CHECKED(MemoryAllocator::Allocators[i].Init(
				kFrameBasedMemSize,
				kFrameBasedMemAlign));
		}

		// Initialize Renderer Backend
		CHECKED(renderer->Init());

		//
		frameNo = 0;
		BeginFrame();

		// Create Built-in Shaders
		CHECKED(InitBuiltinMaterial(kMaterialTypeTest,
			"assets/test_vs.shader",
			"assets/test_ps.shader"
		));

		CHECKED(InitBuiltinMaterial(kMaterialTypeSkybox,
			"assets/skybox_vs.shader",
			"assets/skybox_ps.shader"
		));

		CHECKED(InitBuiltinMaterial(kMaterialTypeOpaque,
			"assets/opaque_vs.shader",
			"assets/opaque_ps.shader"
		));

		CHECKED(InitBuiltinMaterial(kMaterialTypeOpaqueSkinned,
			"assets/opaque_skinned_vs.shader",
			//"assets/opaque_ps.shader"
			"assets/opaque_skinned_ps.shader"
		));

		
		CHECKED(LoadVertexShader("assets/shadow_opaque_vs.shader", materialVSs[kMaterialTypeDepth]));
		CHECKED(LoadVertexShader("assets/shadow_opaqueskinned_vs.shader", materialVSs[kMaterialTypeDepthSkinned]));

		CHECKED(InitBuiltinMaterial(kMaterialDeferredGeometryOpaque,
			"assets/deferred_geomtery_vs.shader",
			"assets/deferred_geomtery_ps.shader"));

		CHECKED(InitBuiltinMaterial(kMaterialDeferredGeometryOpaqueSkinned,
			"assets/deferred_geomtery_skinned_vs.shader",
			"assets/deferred_geomtery_ps.shader"));

		// constant buffers
		{
			transformBuffer = bufferHandleAlloc.Allocate();
			assert(transformBuffer);
			transformBufferSize = sizeof(math::float4x4) * 4 * kMaxEntities;

			{
				CreateBufferParams* params = MemoryAllocator::Allocate<CreateBufferParams>(allocNo);
				assert(nullptr != params);
				params->handle = transformBuffer;
				params->bindingFlags = kBindingConstantBuffer;
				params->size = transformBufferSize;
				params->dynamic = 1;

				cmdBuf->Add(RendererCommand::kCommandCreateBuffer, params);
			}

			frameConstantBuffer = bufferHandleAlloc.Allocate();
			assert(frameConstantBuffer);

			{
				CreateBufferParams* params = MemoryAllocator::Allocate<CreateBufferParams>(allocNo);
				assert(nullptr != params);
				params->handle = frameConstantBuffer;
				params->bindingFlags = kBindingConstantBuffer;
				params->size = sizeof(FrameConstants);
				params->dynamic = 1;

				cmdBuf->Add(RendererCommand::kCommandCreateBuffer, params);
			}

			shadowDepthBuffer = bufferHandleAlloc.Allocate();
			assert(shadowDepthBuffer);

			{
				CreateBufferParams* params = MemoryAllocator::Allocate<CreateBufferParams>(allocNo);
				assert(nullptr != params);
				params->handle = shadowDepthBuffer;
				params->bindingFlags = kBindingConstantBuffer;
				params->size = sizeof(FrameConstants);
				params->dynamic = 1;

				cmdBuf->Add(RendererCommand::kCommandCreateBuffer, params);
			}


			lightingConstantBuffer = bufferHandleAlloc.Allocate();
			assert(lightingConstantBuffer);
			{
				CreateBufferParams* params = MemoryAllocator::Allocate<CreateBufferParams>(allocNo);
				assert(nullptr != params);
				params->handle = lightingConstantBuffer;
				params->bindingFlags = kBindingConstantBuffer;
				params->size = sizeof(LightingConstants);
				params->dynamic = 1;

				cmdBuf->Add(RendererCommand::kCommandCreateBuffer, params);
			}
			
		}

		// create built-in pipeline states
		{
			materialPSOs[kMaterialTypeTest] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[kMaterialTypeTest]);

			{
				CreatePipelineStateParams* params = MemoryAllocator::Allocate<CreatePipelineStateParams>(allocNo);
				params->handle = materialPSOs[kMaterialTypeTest];
				params->vertexShader = materialVSs[kMaterialTypeTest];
				params->pixelShader = materialPSs[kMaterialTypeTest];

				cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
			}

			materialPSOs[kMaterialTypeSkybox] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[kMaterialTypeSkybox]);

			{
				CreatePipelineStateParams* params = MemoryAllocator::Allocate<CreatePipelineStateParams>(allocNo);
				params->handle = materialPSOs[kMaterialTypeSkybox];
				params->vertexShader = materialVSs[kMaterialTypeSkybox];
				params->pixelShader = materialPSs[kMaterialTypeSkybox];
				params->cullMode = kCullFront;
				params->depthFunc = kComparisonAlways;
				cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
			}

			materialPSOs[kMaterialTypeOpaque] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[kMaterialTypeOpaque]);

			{
				CreatePipelineStateParams* params = MemoryAllocator::Allocate<CreatePipelineStateParams>(allocNo);
				params->handle = materialPSOs[kMaterialTypeOpaque];
				params->vertexShader = materialVSs[kMaterialTypeOpaque];
				params->pixelShader = materialPSs[kMaterialTypeOpaque];

				cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
			}

			materialPSOs[kMaterialTypeDepth] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[kMaterialTypeDepth]);

			{
				CreatePipelineStateParams* params = MemoryAllocator::Allocate<CreatePipelineStateParams>(allocNo);
				params->handle = materialPSOs[kMaterialTypeDepth];
				params->vertexShader = materialVSs[kMaterialTypeDepth];
		
				params->cullMode = 1;
				cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
			}

			materialPSOs[kMaterialTypeDepthSkinned] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[kMaterialTypeDepthSkinned]);

			{
				CreatePipelineStateParams* params = MemoryAllocator::Allocate<CreatePipelineStateParams>(allocNo);
				params->handle = materialPSOs[kMaterialTypeDepthSkinned];
				params->vertexShader = materialVSs[kMaterialTypeDepthSkinned];
				params->vertexFormat = kVertexFormatSkinned;
				params->cullMode = 1;
				cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
			}

			materialPSOs[kMaterialTypeOpaqueSkinned] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[kMaterialTypeOpaqueSkinned]);

			{
				CreatePipelineStateParams* params = MemoryAllocator::Allocate<CreatePipelineStateParams>(allocNo);
				params->handle = materialPSOs[kMaterialTypeOpaqueSkinned];
				params->vertexShader = materialVSs[kMaterialTypeOpaqueSkinned];
				params->pixelShader = materialPSs[kMaterialTypeOpaqueSkinned];
				params->vertexFormat = kVertexFormatSkinned;

				cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
			}
		}

		// create default sampler
		{
			defaultSampler = samplerHandleAlloc.Allocate();
			assert(defaultSampler);

			{
				CreateSamplerParams* params = MemoryAllocator::Allocate<CreateSamplerParams>(allocNo);
				params->handle = defaultSampler;
				params->textureAddressU = 1;
				params->textureAddressV = 1;
				params->textureAddressW = 1;
				cmdBuf->Add(RendererCommand::kCommandCreateSampler, params);
			}
		}
		// Create shadow sampler
		{
			shadowSampler = samplerHandleAlloc.Allocate();
			assert(shadowSampler);

			{
				CreateSamplerParams* params = MemoryAllocator::Allocate<CreateSamplerParams>(allocNo);
				params->handle = shadowSampler;
				params->textureAddressU = 3;
				params->textureAddressV = 3;
				params->textureAddressW = 3;

				cmdBuf->Add(RendererCommand::kCommandCreateSampler, params);
			}
		}

		

		builtinCube = CreateModel("assets/cube.model");
		assert(nullptr != builtinCube);

		/*{
			testRT = textureHandleAlloc.Allocate();
			if (!testRT)
				return kErrUnknown;

			CreateTextureParams* params = MemoryAllocator::Allocate<CreateTextureParams>(allocNo);
			params->handle = testRT;
			params->format = kFormatR8g8b8a8Unorm;
			params->arraySize = 1;
			params->bindingFlags = kBindingShaderResource | kBindingRenderTarget;
			
			int32_t w, h;
			renderer->GetFrameBufferSize(w, h);
			params->width = w;
			params->height = h;

			cmdBuf->Add(RendererCommand::kCommandCreateTexture, params);
		}*/

		// TODO when back buffer size changed !
		int32_t w, h;
		renderer->GetFrameBufferSize(w, h);
		gBufferPosition = CreateTexture(kFormatR32g32b32a32Float, w, h, 0, nullptr, kBindingRenderTarget | kBindingShaderResource);
		gBufferNormal = CreateTexture(kFormatR32g32b32a32Float, w, h, 0, nullptr, kBindingRenderTarget | kBindingShaderResource);
		gBufferAlbedo = CreateTexture(kFormatR32g32b32a32Float, w, h, 0, nullptr, kBindingRenderTarget | kBindingShaderResource);
		gBufferDepth = CreateTexture(kFormatD24UnormS8Uint, w, h, 0, nullptr, kBindingDepthStencil | kBindingShaderResource);

		if (!gBufferPosition || !gBufferNormal || !gBufferAlbedo || !gBufferDepth)
		{
			return kErrUnknown;
		}

		return kOK;
	}

	int32_t RenderingSystem::Shutdown()
	{
		renderer->Release();

		for (uint32_t i = kAllocFrameBasedMem;
			i <= kAllocFrameBasedMemEnd;
			++i)
		{
			CHECKED(MemoryAllocator::Allocators[i].Shutdown());
		}

		CHECKED(MemoryAllocator::Allocators[kAllocLevelBasedMem].Shutdown());
		return kOK;
	}

	int32_t RenderingSystem::BeginFrame()
	{
		if (nullptr != cmdBuf)
		{
			return kOK;
		}

		//assert(false && "sync need to be done.");

		allocNo = kAllocFrameBasedMem + frameNo % kFrameBufferCount;
		MemoryAllocator::Allocators[allocNo].Reset();

		cmdBuf = RendererCommandBuffer::Create(kCommandBufferCapacity, allocNo);
		assert(nullptr != cmdBuf);

		return kOK;
	}

	int32_t RenderingSystem::Update()
	{
		return DeferredPipeline();

		if (CameraComponent::GetNumComponents() == 0)
		{
			return kOK;
		}
		CameraComponentData& camera = CameraComponent::GetAllComponents()[0];

		// update aspect
		{
			int32_t w, h;
			renderer->GetFrameBufferSize(w, h);
			camera.SetAspect(h == 0 ? 1.0f : float(w) / h);
		}
		

		math::float3 camPos;
		{
			FrameConstants* data = reinterpret_cast<FrameConstants*>(
				MemoryAllocator::Allocators[allocNo].Allocate(sizeof(FrameConstants), 4)
				);
			assert(nullptr != data);

#ifdef TOFU_USE_GLM
			data->matView = math::transpose(camera.CalcViewMatrix());
			data->matProj = math::transpose(camera.CalcProjectionMatrix());
#else
			data->matView = camera.CalcViewMatrix();
			data->matProj = camera.CalcProjectionMatrix();
#endif

			TransformComponent t = camera.entity.GetComponent<TransformComponent>();
			data->cameraPos = t->GetWorldPosition();

			camPos = data->cameraPos;

			UpdateBufferParams* params = MemoryAllocator::Allocate<UpdateBufferParams>(allocNo);
			assert(nullptr != params);
			params->handle = frameConstantBuffer;
			params->data = data;
			params->size = sizeof(FrameConstants);

			cmdBuf->Add(RendererCommand::kCommandUpdateBuffer, params);
		}
		
		LightComponentData* lights = LightComponent::GetAllComponents();
		uint32_t lightsCount = LightComponent::GetNumComponents();
		//TextureHandle * depthMap = new TextureHandle[lightsCount];
		// Light constant buffer data
		{
			LightingConstants* data = reinterpret_cast<LightingConstants*>(
					MemoryAllocator::Allocators[allocNo].Allocate(sizeof(LightingConstants), 16)
					);
				assert(nullptr != data);
				
				for (uint32_t i = 0; i < lightsCount; ++i)
				{
					LightComponentData& comp = lights[i];

					// TODO culling
                    TransformComponent t = comp.entity.GetComponent<TransformComponent>();
					if (comp.type == kLightTypeDirectional) {
                        data->lightDirection[i] = math::float4{ (t->GetForwardVector()).x,
						(t->GetForwardVector()).y,
						( t->GetForwardVector()).z ,0.0f };
						data->type[i].x = 1.0f;
					}
					else if (comp.type == kLightTypePoint) {
						data->type[i].x = 2.0f;
						data->lightPosition[i] = math::float4(t->GetWorldPosition(),0);
					}
					else {
						data->type[i].x = 3.0f;
					}
	                data->lightColor[i] = comp.lightColor;
				}
				data->count = lightsCount;
				data->camPos = camPos;
				

				UpdateBufferParams* params = MemoryAllocator::Allocate<UpdateBufferParams>(allocNo);
				assert(nullptr != params);
				params->handle = lightingConstantBuffer;
				params->data = data;
				params->size = sizeof(LightingConstants);

				cmdBuf->Add(RendererCommand::kCommandUpdateBuffer, params);
		}

		// clear
		TextureHandle skyboxTex = TextureHandle();

		if (nullptr == camera.skybox)
		{
			ClearParams* params = MemoryAllocator::Allocate<ClearParams>(allocNo);

			const math::float4& clearColor = camera.GetClearColor();

			params->clearColor[0] = clearColor.x;
			params->clearColor[1] = clearColor.y;
			params->clearColor[2] = clearColor.z;
			params->clearColor[3] = clearColor.w;

			cmdBuf->Add(RendererCommand::kCommandClearRenderTargets, params);
		}
		else
		{
			assert(camera.skybox->type == kMaterialTypeSkybox);
			skyboxTex = camera.skybox->mainTex;

			Mesh& mesh = meshes[builtinCube->meshes[0].id];

			DrawParams* params = MemoryAllocator::Allocate<DrawParams>(allocNo);
			params->pipelineState = materialPSOs[kMaterialTypeSkybox];
			params->vertexBuffer = mesh.VertexBuffer;
			params->indexBuffer = mesh.IndexBuffer;
			params->startIndex = mesh.StartIndex;
			params->startVertex = mesh.StartVertex;
			params->indexCount = mesh.NumIndices;
			params->vsConstantBuffers[0] = { frameConstantBuffer, 0, 16 };

			params->psTextures[0] = skyboxTex;
			params->psSamplers[0] = defaultSampler;

			cmdBuf->Add(RendererCommand::kCommandDraw, params);
		}

		// get all renderables in system
		RenderingComponentData* renderables = RenderingComponent::GetAllComponents();
		uint32_t renderableCount = RenderingComponent::GetNumComponents();

		// buffer for transform matrices
		math::float4x4* transformArray = reinterpret_cast<math::float4x4*>(
			MemoryAllocator::Allocators[allocNo].Allocate(transformBufferSize, 4)
			);

		// list of active renderables (used for culling)
		uint32_t* activeRenderables = reinterpret_cast<uint32_t*>(
			MemoryAllocator::Allocators[allocNo].Allocate(sizeof(uint32_t) * kMaxEntities, 4)
			);

		assert(nullptr != transformArray && nullptr != activeRenderables);

		uint32_t numActiveRenderables = 0;

		// fill in transform matrix for active renderables
		for (uint32_t i = 0; i < renderableCount; ++i)
	   // for (uint32_t i = renderableCount - 2; i < renderableCount; ++i)
		{
			RenderingComponentData& comp = renderables[i];
			TransformComponent transform = comp.entity.GetComponent<TransformComponent>();
			assert(transform);

			// TODO culling

			uint32_t idx = numActiveRenderables++;
			activeRenderables[idx] = i;

#ifdef TOFU_USE_GLM
			transformArray[idx * 4] = math::transpose(transform->GetWorldTransform().GetMatrix());
#else
			transformArray[idx * 4] = transform->GetWorldTransform().GetMatrix();
#endif
		}

		// upload transform matrices
		if (numActiveRenderables > 0)
		{
			UpdateBufferParams* params = MemoryAllocator::Allocate<UpdateBufferParams>(allocNo);
			assert(nullptr != params);
			params->handle = transformBuffer;
			params->data = transformArray;
			params->size = sizeof(math::float4x4) * 4 * numActiveRenderables;

			cmdBuf->Add(RendererCommand::kCommandUpdateBuffer, params);
		}

		// update skinned mesh animation bone matrices
		AnimationComponentData* animComps = AnimationComponent::GetAllComponents();
		uint32_t animCompCount = AnimationComponent::GetNumComponents();

		for (uint32_t i = 0; i < animCompCount; i++)
		{
			AnimationComponentData& anim = animComps[i];
			RenderingComponent r = anim.entity.GetComponent<RenderingComponent>();

			if (!r || nullptr == r->model || !r->model->HasAnimation())
			{
				return kErrUnknown;
			}

			// re-alloc constant buffer if model changed (or firstly set)
			if (anim.model != r->model)
			{
				anim.model = r->model;
				
				int32_t err = ReallocAnimationResources(anim);
				if (kOK != err)
				{
					return err;
				}
			}

			// update and fill in bone matrices
			void* boneMatrices = MemoryAllocator::Allocators[allocNo].Allocate(anim.boneMatricesBufferSize, 4);

			anim.UpdateTiming();
			anim.FillInBoneMatrices(boneMatrices, anim.boneMatricesBufferSize);

			// upload bone matrices
			UpdateBufferParams* params = MemoryAllocator::Allocate<UpdateBufferParams>(allocNo);
			assert(nullptr != params);
			params->handle = anim.boneMatricesBuffer;
			params->data = boneMatrices;
			params->size = anim.boneMatricesBufferSize;

			cmdBuf->Add(RendererCommand::kCommandUpdateBuffer, params);
		}


		// Generate shadow map
		// So far one light
		for (uint32_t i = 0; i < 1; ++i) {

			FrameConstants* data = reinterpret_cast<FrameConstants*>(
				MemoryAllocator::Allocators[allocNo].Allocate(sizeof(FrameConstants), 4)
				);
			assert(nullptr != data);
			
            TransformComponent t = lights[i].entity.GetComponent<TransformComponent>();
			data->cameraPos = camPos;

			
			math::float3 lightPos = camPos;
			//math::float3 lightPos = t->GetWorldPosition();
			math::float3 forward = t->GetForwardVector();
			math::float4x4 lightView = math::lookTo(lightPos, forward, math::float3{ 0, 1, 0 });
		//	math::float4x4 lightProject = math::perspective(90.0f * 3.1415f / 180.0f, 1.0f, 0.01f, 100.0f);
			math::float4x4 lightProject = math::orthoLH(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 100.0f);
#ifdef TOFU_USE_GLM
			data->matView = math::transpose(lightView);
			data->matProj = math::transpose(lightProject);
#else
			data->matView = camera.CalcViewMatrix();
			data->matProj = camera.CalcProjectionMatrix();
#endif
			UpdateBufferParams* params = MemoryAllocator::Allocate<UpdateBufferParams>(allocNo);
			assert(nullptr != params);
			params->handle = shadowDepthBuffer;
		//	params->handle = frameConstantBuffer;
			params->data = data;
			params->size = sizeof(FrameConstants);

			cmdBuf->Add(RendererCommand::kCommandUpdateBuffer, params);
		}
		{
			ClearParams* params = MemoryAllocator::Allocate<ClearParams>(allocNo);
			params->renderTargets[0] = TextureHandle();
			params->depthRenderTarget = lights[0].depthMap;
			params->clearDepth = 1.0f;
			params->clearStencil = 100.0f;

			cmdBuf->Add(RendererCommand::kCommandClearRenderTargets, params);
		}
		for (uint32_t i = 0; i < 1; ++i)
		{
			RenderingComponentData& comp = renderables[activeRenderables[i]];

			assert(nullptr != comp.model);
			assert(0 != comp.numMaterials);

			Model& model = *comp.model;
			//Material* mat = comp.material;

			for (uint32_t iMesh = 0; iMesh < model.numMeshes; ++iMesh)
			{
				assert(model.meshes[iMesh]);
				Mesh& mesh = meshes[model.meshes[iMesh].id];
				Material* mat = comp.materials[iMesh < comp.numMaterials ? iMesh : (comp.numMaterials - 1)];

				DrawParams* params = MemoryAllocator::Allocate<DrawParams>(allocNo);
				params->pipelineState = materialPSOs[kMaterialTypeDepth];
				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;
				params->renderTargets[0] = TextureHandle();
				params->depthRenderTarget = lights[0].depthMap;


				params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(i * 16), 16 };
				params->vsConstantBuffers[1] = { shadowDepthBuffer, 0, 16 };
				//params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 16 };

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}
		}

		for (uint32_t i = numActiveRenderables - 1; i < numActiveRenderables; ++i)
		{
			RenderingComponentData& comp = renderables[activeRenderables[i]];

			assert(nullptr != comp.model);
			assert(0 != comp.numMaterials);

			Model& model = *comp.model;
			//Material* mat = comp.material;

			for (uint32_t iMesh = 0; iMesh < model.numMeshes; ++iMesh)
			{
				assert(model.meshes[iMesh]);
				Mesh& mesh = meshes[model.meshes[iMesh].id];
				Material* mat = comp.materials[iMesh < comp.numMaterials ? iMesh : (comp.numMaterials - 1)];

				DrawParams* params = MemoryAllocator::Allocate<DrawParams>(allocNo);
				params->pipelineState = materialPSOs[kMaterialTypeOpaqueSkinned];
				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;
				params->renderTargets[0] = TextureHandle();
				params->depthRenderTarget = lights[0].depthMap;
			
				params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(i * 16), 16 };
				params->vsConstantBuffers[1] = { shadowDepthBuffer, 0, 16 };
				//params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 16 };

				(void*)0;
				{
					AnimationComponent anim = comp.entity.GetComponent<AnimationComponent>();
					if (!anim || !anim->boneMatricesBuffer)
					{
						return kErrUnknown;
					}
					params->vsConstantBuffers[2] = { anim->boneMatricesBuffer, 0, 0 };
				}

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}
		}
		

		// generate draw call for active renderables in command buffer
		for (uint32_t i = 0; i < numActiveRenderables; ++i)
	//	for (uint32_t i = 0; i < 1; ++i)
		{
			RenderingComponentData& comp = renderables[activeRenderables[i]];

			assert(nullptr != comp.model);
			assert(0 != comp.numMaterials);

			Model& model = *comp.model;
			//Material* mat = comp.material;

			for (uint32_t iMesh = 0; iMesh < model.numMeshes; ++iMesh)
			{
				assert(model.meshes[iMesh]);
				Mesh& mesh = meshes[model.meshes[iMesh].id];
				Material* mat = comp.materials[iMesh < comp.numMaterials ? iMesh : (comp.numMaterials - 1)];

				DrawParams* params = MemoryAllocator::Allocate<DrawParams>(allocNo);
				params->pipelineState = materialPSOs[mat->type];
				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;
				

				switch (mat->type)
				{
				case kMaterialTypeTest:
					params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(i * 16), 16 };
					params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 16 };
					params->psTextures[0] = mat->mainTex;
					params->psSamplers[0] = defaultSampler;
					break;
				case kMaterialTypeOpaqueSkinned:
					(void*)0;
					{
						AnimationComponent anim = comp.entity.GetComponent<AnimationComponent>();
						if (!anim || !anim->boneMatricesBuffer)
						{
							return kErrUnknown;
						}
						params->vsConstantBuffers[2] = { anim->boneMatricesBuffer, 0, 0 };
					}
					params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(i * 16), 16 };
					params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 16 };
				//	params->vsConstantBuffers[1] = { shadowDepthBuffer, 0, 16 };
					params->psConstantBuffers[0] = { lightingConstantBuffer,0, 4096 };
					params->psTextures[0] = skyboxTex;
					params->psTextures[1] = mat->mainTex;
					params->psTextures[2] = mat->normalMap;
					params->psSamplers[0] = defaultSampler;
					
					break;
					// fall through
				case kMaterialTypeOpaque:
					params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(i * 16), 16 };
					params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 16 };
					params->vsConstantBuffers[2] = { shadowDepthBuffer, 0, 16 };
				//	params->vsConstantBuffers[1] = { shadowDepthBuffer, 0, 16 };
					params->psConstantBuffers[0] = { lightingConstantBuffer,0, 4096 };
					params->psTextures[0] = skyboxTex;
					params->psTextures[1] = mat->mainTex;
					params->psTextures[2] = mat->normalMap;
			     	params->psTextures[3] = lights[0].depthMap;
					params->psSamplers[0] = defaultSampler;
					params->psSamplers[1] = shadowSampler;
					break;
				default:
					assert(false && "this material type is not applicable for entities");
					break;
				}

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}
		}

		return kOK;
	}

	int32_t RenderingSystem::EndFrame()
	{
		// submit command buffer
		CHECKED(renderer->Submit(cmdBuf));

		// back buffer swap
		renderer->Present();

		frameNo++;

		cmdBuf = nullptr;

		return kOK;
	}

	Model* RenderingSystem::CreateModel(const char* filename)
	{
		std::string strFilename(filename);

		{
			auto iter = modelTable.find(strFilename);
			if (iter != modelTable.end())
			{
				return &models[iter->second.id];
			}
		}

		uint8_t* data = nullptr;
		size_t size = 0u;
		int32_t err = FileIO::ReadFile(filename, false, 4, kAllocFrameBasedMem, reinterpret_cast<void**>(&data), &size);
		if (kOK != err)
		{
			return nullptr;
		}

		// read header
		model::ModelHeader* header = reinterpret_cast<model::ModelHeader*>(data);

		if (header->Magic != model::kModelFileMagic)
		{
			return nullptr;
		}
		assert(header->StructOfArray == 0);
		assert(header->HasIndices == 1);
		assert(header->HasTangent == 1);
		assert(header->NumTexcoordChannels == 1);

		if (header->NumMeshes == 0)
		{
			return nullptr;
		}

		// get mesh info list
		model::ModelMesh* meshInfos = reinterpret_cast<model::ModelMesh*>(header + 1);

		uint32_t verticesCount = 0;
		uint32_t indicesCount = 0;

		assert(header->NumMeshes <= kMaxMeshesPerModel);
		ModelHandle modelHandle = modelHandleAlloc.Allocate();
		assert(modelHandle);
		Model& model = models[modelHandle.id];

		model = Model();
		model.handle = modelHandle;
		model.numMeshes = header->NumMeshes;
		model.vertexSize = header->CalculateVertexSize();
		model.rawData = data;
		model.rawDataSize = size;
		model.header = header;

		// allocate vertex buffer and index buffer
		BufferHandle vbHandle = bufferHandleAlloc.Allocate();
		BufferHandle ibHandle = bufferHandleAlloc.Allocate();
		assert(vbHandle && ibHandle);

		// store mesh infos
		for (uint32_t i = 0; i < header->NumMeshes; ++i)
		{
			model.meshes[i] = meshHandleAlloc.Allocate();
			assert(model.meshes[i]);
			uint32_t id = model.meshes[i].id;
			meshes[id] = Mesh();
			meshes[id].VertexBuffer = vbHandle;
			meshes[id].IndexBuffer = ibHandle;
			meshes[id].StartVertex = verticesCount;
			meshes[id].StartIndex = indicesCount;
			meshes[id].NumVertices = meshInfos[i].NumVertices;
			meshes[id].NumIndices = meshInfos[i].NumIndices;

			verticesCount += meshInfos[i].NumVertices;
			indicesCount += meshInfos[i].NumIndices;
		}

		// aligned to dword
		if (indicesCount % 2 != 0)
		{
			indicesCount += 1;
		}

		// 
		uint32_t vertexBufferSize = verticesCount * header->CalculateVertexSize();
		uint32_t indexBufferSize = indicesCount * sizeof(uint16_t);

		uint8_t* vertices = reinterpret_cast<uint8_t*>(meshInfos + header->NumMeshes);
		uint8_t* indices = vertices + vertexBufferSize;

		for (uint32_t i = 0; i < header->NumMeshes; ++i)
		{
			uint32_t id = model.meshes[i].id;
			model.numVertices[i] = meshes[id].NumVertices;
			model.numIndices[i] = meshes[id].NumIndices;
			model.vertices[i] = reinterpret_cast<float*>(
				vertices + meshes[id].StartVertex * model.vertexSize);
			model.indices[i] = reinterpret_cast<uint16_t*>(
				indices + meshes[id].StartIndex * sizeof(uint16_t));
		}

		// keep pointers to bone and animation structures
		if (header->NumBones > 0)
		{
			model.bones = reinterpret_cast<model::ModelBone*>(indices + indexBufferSize);
			
#ifdef TOFU_USE_GLM
			// transpose matrix, model convertor is using row-major layout
			for (uint32_t iBone = 0; iBone < header->NumBones; iBone++)
			{
				model.bones[iBone].transform = math::transpose(model.bones[iBone].transform);
				model.bones[iBone].offsetMatrix = math::transpose(model.bones[iBone].offsetMatrix);
			}
#endif
			
			if (header->HasAnimation)
			{
				model.animations = reinterpret_cast<model::ModelAnimation*>(
					model.bones + header->NumBones
					);

				for (auto i = 0u; i < header->NumAnimations; i++) {
					model.animationTable[model.animations[i].name] = i;
				}

				// FIXME: Test only
				model.animationTable["idle"] = 0;
				model.animationTable["walk"] = 1;
				model.animationTable["jump"] = 2;
				model.animationTable["run"] = 3;

				model.frames = reinterpret_cast<model::ModelAnimFrame*>(
					model.animations + header->NumAnimations
					);
			}
		}

		// upload vertices and indices to vertex buffer and index buffer
		{
			CreateBufferParams* params = MemoryAllocator::Allocate<CreateBufferParams>(kAllocFrameBasedMem);
			params->handle = vbHandle;
			params->bindingFlags = kBindingVertexBuffer;
			params->data = vertices;
			params->size = vertexBufferSize;
			params->stride = header->CalculateVertexSize();

			cmdBuf->Add(RendererCommand::kCommandCreateBuffer, params);
		}

		{
			CreateBufferParams* params = MemoryAllocator::Allocate<CreateBufferParams>(kAllocFrameBasedMem);
			params->handle = ibHandle;
			params->bindingFlags = kBindingIndexBuffer;
			params->data = indices;
			params->size = indexBufferSize;

			cmdBuf->Add(RendererCommand::kCommandCreateBuffer, params);
		}

		modelTable.insert(std::pair<std::string, ModelHandle>(strFilename, modelHandle));

		return &model;
	}

	TextureHandle RenderingSystem::CreateTexture(const char* filename)
	{
		void* data = nullptr;
		size_t size = 0;
		int32_t err = FileIO::ReadFile(filename, false, 4, allocNo, &data, &size);

		if (kOK != err)
		{
			return TextureHandle();
		}

		TextureHandle handle = textureHandleAlloc.Allocate();
		if (!handle)
		{
			return TextureHandle();
		}

		CreateTextureParams* params = MemoryAllocator::Allocate<CreateTextureParams>(allocNo);

		params->handle = handle;
		params->bindingFlags = kBindingShaderResource;
		params->isFile = 1;
		params->data = data;
		params->width = static_cast<uint32_t>(size);

		cmdBuf->Add(RendererCommand::kCommandCreateTexture, params);

		return handle;
	}

	TextureHandle RenderingSystem::CreateTexture(PixelFormat format, uint32_t width, uint32_t height, uint32_t pitch, void* data, uint32_t binding)
	{
		TextureHandle handle = textureHandleAlloc.Allocate();
		if (!handle)
		{
			return handle;
		}

		CreateTextureParams* params = MemoryAllocator::Allocate<CreateTextureParams>(allocNo);

		params->handle = handle;
		params->bindingFlags = binding;
		params->format = format;
		params->arraySize = 1;
		params->width = width;
		params->height = height;
		params->pitch = pitch;
		params->data = data;

		cmdBuf->Add(RendererCommand::kCommandCreateTexture, params);

		return handle;
	}

	Material* RenderingSystem::CreateMaterial(MaterialType type)
	{
		MaterialHandle handle = materialHandleAlloc.Allocate();
		if (!handle)
		{
			return nullptr;
		}

		Material* mat = &(materials[handle.id]);
		new (mat) Material(type);
		mat->handle = handle;

		return mat;
	}

	TextureHandle RenderingSystem::CreateDepthMap( uint32_t width, uint32_t height)
	{
		TextureHandle handle = textureHandleAlloc.Allocate();
		if (!handle)
		{
			return handle;
		}

		CreateTextureParams* params = MemoryAllocator::Allocate<CreateTextureParams>(allocNo);
		params->handle = handle;
		params->format = kFormatD24UnormS8Uint;
 		params->arraySize = 1;
		params->bindingFlags = kBindingShaderResource | kBindingDepthStencil;
		params->width = width;
		params->height = height;

		cmdBuf->Add(RendererCommand::kCommandCreateTexture, params);
		
		return handle;
	}

	int32_t RenderingSystem::InitBuiltinMaterial(MaterialType matType, const char * vsFile, const char * psFile)
	{
		if (materialVSs[matType] || materialPSs[matType])
			return kErrUnknown;
		
		CHECKED(LoadVertexShader(vsFile, materialVSs[matType]));
		CHECKED(LoadPixelShader(psFile, materialPSs[matType]));

		return kOK;
	}

	int32_t RenderingSystem::LoadVertexShader(const char * filename, VertexShaderHandle & handle)
	{
		handle = vertexShaderHandleAlloc.Allocate();
		if (!handle)
		{
			return kErrUnknown;
		}

		{
			CreateVertexShaderParams* params = MemoryAllocator::Allocate<CreateVertexShaderParams>(allocNo);
			assert(nullptr != params);
			params->handle = handle;

			int32_t err = FileIO::ReadFile(
				filename,
				false,
				4,
				allocNo,
				&(params->data),
				&(params->size));

			if (kOK != err)
			{
				return err;
			}

			cmdBuf->Add(RendererCommand::kCommandCreateVertexShader, params);
		}

		return kOK;
	}

	int32_t RenderingSystem::LoadPixelShader(const char * filename, PixelShaderHandle & handle)
	{
		handle = pixelShaderHandleAlloc.Allocate();
		if (!handle)
		{
			return kErrUnknown;
		}

		{
			CreatePixelShaderParams* params = MemoryAllocator::Allocate<CreatePixelShaderParams>(allocNo);
			assert(nullptr != params);
			params->handle = handle;

			int32_t err = FileIO::ReadFile(
				filename,
				false,
				4,
				allocNo,
				&(params->data),
				&(params->size));

			if (kOK != err)
			{
				return err;
			}

			cmdBuf->Add(RendererCommand::kCommandCreatePixelShader, params);
		}

		return kOK;
	}

	int32_t RenderingSystem::ReallocAnimationResources(AnimationComponentData & c)
	{
		if (c.boneMatricesBuffer)
		{
			// TODO dealloc
		}

		// TODO:
		c.ReallocResources();

		Model* model = c.model;
		if (nullptr == model || nullptr == model->header || !model->HasAnimation() || 0 == model->header->NumBones)
		{
			return kErrUnknown;
		}

		BufferHandle bufferHandle = bufferHandleAlloc.Allocate();
		if (!bufferHandle)
		{
			return kErrUnknown;
		}

		c.boneMatricesBuffer = bufferHandle;
		c.boneMatricesBufferSize = static_cast<uint32_t>(sizeof(math::float4x4)) * model->header->NumBones;

		CreateBufferParams* params = MemoryAllocator::Allocate<CreateBufferParams>(allocNo);
		assert(nullptr != params);
		params->handle = bufferHandle;
		params->bindingFlags = kBindingConstantBuffer;
		params->size = c.boneMatricesBufferSize;
		params->dynamic = 1;

		cmdBuf->Add(RendererCommand::kCommandCreateBuffer, params);

		return kOK;
	}

	int32_t RenderingSystem::DeferredPipeline()
	{
		if (CameraComponent::GetNumComponents() == 0)
		{
			return kOK;
		}
		CameraComponentData& camera = CameraComponent::GetAllComponents()[0];

		// update aspect
		{
			int32_t w, h;
			renderer->GetFrameBufferSize(w, h);
			camera.SetAspect(h == 0 ? 1.0f : float(w) / h);
		}


		math::float3 camPos;
		{
			FrameConstants* data = reinterpret_cast<FrameConstants*>(
				MemoryAllocator::Allocators[allocNo].Allocate(sizeof(FrameConstants), 4)
				);
			assert(nullptr != data);

#ifdef TOFU_USE_GLM
			data->matView = math::transpose(camera.CalcViewMatrix());
			data->matProj = math::transpose(camera.CalcProjectionMatrix());
#else
			data->matView = camera.CalcViewMatrix();
			data->matProj = camera.CalcProjectionMatrix();
#endif

			TransformComponent t = camera.entity.GetComponent<TransformComponent>();
			data->cameraPos = t->GetWorldPosition();

			camPos = data->cameraPos;

			UpdateBufferParams* params = MemoryAllocator::Allocate<UpdateBufferParams>(allocNo);
			assert(nullptr != params);
			params->handle = frameConstantBuffer;
			params->data = data;
			params->size = sizeof(FrameConstants);

			cmdBuf->Add(RendererCommand::kCommandUpdateBuffer, params);
		}

		//LightComponentData* lights = LightComponent::GetAllComponents();
		//uint32_t lightsCount = LightComponent::GetNumComponents();
		////TextureHandle * depthMap = new TextureHandle[lightsCount];
		//// Light constant buffer data
		//{
		//	LightingConstants* data = reinterpret_cast<LightingConstants*>(
		//		MemoryAllocator::Allocators[allocNo].Allocate(sizeof(LightingConstants), 16)
		//		);
		//	assert(nullptr != data);

		//	for (uint32_t i = 0; i < lightsCount; ++i)
		//	{
		//		LightComponentData& comp = lights[i];

		//		// TODO culling
		//		TransformComponent t = comp.entity.GetComponent<TransformComponent>();
		//		if (comp.type == kLightTypeDirectional) {
		//			data->lightDirection[i] = math::float4{ (t->GetForwardVector()).x,
		//				(t->GetForwardVector()).y,
		//				(t->GetForwardVector()).z ,0.0f };
		//			data->type[i].x = 1.0f;
		//		}
		//		else if (comp.type == kLightTypePoint) {
		//			data->type[i].x = 2.0f;
		//			data->lightPosition[i] = math::float4(t->GetWorldPosition(), 0);
		//		}
		//		else {
		//			data->type[i].x = 3.0f;
		//		}
		//		data->lightColor[i] = comp.lightColor;
		//	}
		//	data->count = lightsCount;
		//	data->camPos = camPos;


		//	UpdateBufferParams* params = MemoryAllocator::Allocate<UpdateBufferParams>(allocNo);
		//	assert(nullptr != params);
		//	params->handle = lightingConstantBuffer;
		//	params->data = data;
		//	params->size = sizeof(LightingConstants);

		//	cmdBuf->Add(RendererCommand::kCommandUpdateBuffer, params);
		//}

		// clear
		TextureHandle skyboxTex = TextureHandle();

		if (nullptr == camera.skybox)
		{
			ClearParams* params = MemoryAllocator::Allocate<ClearParams>(allocNo);

			const math::float4& clearColor = camera.GetClearColor();

			params->clearColor[0] = clearColor.x;
			params->clearColor[1] = clearColor.y;
			params->clearColor[2] = clearColor.z;
			params->clearColor[3] = clearColor.w;

			cmdBuf->Add(RendererCommand::kCommandClearRenderTargets, params);
		}
		else
		{
			assert(camera.skybox->type == kMaterialTypeSkybox);
			skyboxTex = camera.skybox->mainTex;

			Mesh& mesh = meshes[builtinCube->meshes[0].id];

			DrawParams* params = MemoryAllocator::Allocate<DrawParams>(allocNo);
			params->pipelineState = materialPSOs[kMaterialTypeSkybox];
			params->vertexBuffer = mesh.VertexBuffer;
			params->indexBuffer = mesh.IndexBuffer;
			params->startIndex = mesh.StartIndex;
			params->startVertex = mesh.StartVertex;
			params->indexCount = mesh.NumIndices;
			params->vsConstantBuffers[0] = { frameConstantBuffer, 0, 16 };

			params->psTextures[0] = skyboxTex;
			params->psSamplers[0] = defaultSampler;

			cmdBuf->Add(RendererCommand::kCommandDraw, params);
		}

		// get all renderables in system
		RenderingComponentData* renderables = RenderingComponent::GetAllComponents();
		uint32_t renderableCount = RenderingComponent::GetNumComponents();

		// buffer for transform matrices
		math::float4x4* transformArray = reinterpret_cast<math::float4x4*>(
			MemoryAllocator::Allocators[allocNo].Allocate(transformBufferSize, 4)
			);

		// list of active renderables (used for culling)
		uint32_t* activeRenderables = reinterpret_cast<uint32_t*>(
			MemoryAllocator::Allocators[allocNo].Allocate(sizeof(uint32_t) * kMaxEntities, 4)
			);

		assert(nullptr != transformArray && nullptr != activeRenderables);

		uint32_t numActiveRenderables = 0;

		// fill in transform matrix for active renderables
		for (uint32_t i = 0; i < renderableCount; ++i)
			// for (uint32_t i = renderableCount - 2; i < renderableCount; ++i)
		{
			RenderingComponentData& comp = renderables[i];
			TransformComponent transform = comp.entity.GetComponent<TransformComponent>();
			assert(transform);

			// TODO culling

			uint32_t idx = numActiveRenderables++;
			activeRenderables[idx] = i;

#ifdef TOFU_USE_GLM
			transformArray[idx * 4] = math::transpose(transform->GetWorldTransform().GetMatrix());
#else
			transformArray[idx * 4] = transform->GetWorldTransform().GetMatrix();
#endif
		}

		// upload transform matrices
		if (numActiveRenderables > 0)
		{
			UpdateBufferParams* params = MemoryAllocator::Allocate<UpdateBufferParams>(allocNo);
			assert(nullptr != params);
			params->handle = transformBuffer;
			params->data = transformArray;
			params->size = sizeof(math::float4x4) * 4 * numActiveRenderables;

			cmdBuf->Add(RendererCommand::kCommandUpdateBuffer, params);
		}

		// update skinned mesh animation bone matrices
		AnimationComponentData* animComps = AnimationComponent::GetAllComponents();
		uint32_t animCompCount = AnimationComponent::GetNumComponents();

		for (uint32_t i = 0; i < animCompCount; i++)
		{
			AnimationComponentData& anim = animComps[i];
			RenderingComponent r = anim.entity.GetComponent<RenderingComponent>();

			if (!r || nullptr == r->model || !r->model->HasAnimation())
			{
				return kErrUnknown;
			}

			// re-alloc constant buffer if model changed (or firstly set)
			if (anim.model != r->model)
			{
				anim.model = r->model;

				int32_t err = ReallocAnimationResources(anim);
				if (kOK != err)
				{
					return err;
				}
			}

			// update and fill in bone matrices
			void* boneMatrices = MemoryAllocator::Allocators[allocNo].Allocate(anim.boneMatricesBufferSize, 4);

			anim.UpdateTiming();
			anim.FillInBoneMatrices(boneMatrices, anim.boneMatricesBufferSize);

			// upload bone matrices
			UpdateBufferParams* params = MemoryAllocator::Allocate<UpdateBufferParams>(allocNo);
			assert(nullptr != params);
			params->handle = anim.boneMatricesBuffer;
			params->data = boneMatrices;
			params->size = anim.boneMatricesBufferSize;

			cmdBuf->Add(RendererCommand::kCommandUpdateBuffer, params);
		}

//
//		// Generate shadow map
//		// So far one light
//		for (uint32_t i = 0; i < 1; ++i) {
//
//			FrameConstants* data = reinterpret_cast<FrameConstants*>(
//				MemoryAllocator::Allocators[allocNo].Allocate(sizeof(FrameConstants), 4)
//				);
//			assert(nullptr != data);
//
//			TransformComponent t = lights[i].entity.GetComponent<TransformComponent>();
//			data->cameraPos = camPos;
//
//
//			math::float3 lightPos = camPos;
//			//math::float3 lightPos = t->GetWorldPosition();
//			math::float3 forward = t->GetForwardVector();
//			math::float4x4 lightView = math::lookTo(lightPos, forward, math::float3{ 0, 1, 0 });
//			//	math::float4x4 lightProject = math::perspective(90.0f * 3.1415f / 180.0f, 1.0f, 0.01f, 100.0f);
//			math::float4x4 lightProject = math::orthoLH(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 100.0f);
//#ifdef TOFU_USE_GLM
//			data->matView = math::transpose(lightView);
//			data->matProj = math::transpose(lightProject);
//#else
//			data->matView = camera.CalcViewMatrix();
//			data->matProj = camera.CalcProjectionMatrix();
//#endif
//			UpdateBufferParams* params = MemoryAllocator::Allocate<UpdateBufferParams>(allocNo);
//			assert(nullptr != params);
//			params->handle = shadowDepthBuffer;
//			//	params->handle = frameConstantBuffer;
//			params->data = data;
//			params->size = sizeof(FrameConstants);
//
//			cmdBuf->Add(RendererCommand::kCommandUpdateBuffer, params);
//		}
//		{
//			ClearParams* params = MemoryAllocator::Allocate<ClearParams>(allocNo);
//			params->renderTargets[0] = TextureHandle();
//			params->depthRenderTarget = lights[0].depthMap;
//			params->clearDepth = 1.0f;
//			params->clearStencil = 100.0f;
//
//			cmdBuf->Add(RendererCommand::kCommandClearRenderTargets, params);
//		}
//		for (uint32_t i = 0; i < 1; ++i)
//		{
//			RenderingComponentData& comp = renderables[activeRenderables[i]];
//
//			assert(nullptr != comp.model);
//			assert(0 != comp.numMaterials);
//
//			Model& model = *comp.model;
//			//Material* mat = comp.material;
//
//			for (uint32_t iMesh = 0; iMesh < model.numMeshes; ++iMesh)
//			{
//				assert(model.meshes[iMesh]);
//				Mesh& mesh = meshes[model.meshes[iMesh].id];
//				Material* mat = comp.materials[iMesh < comp.numMaterials ? iMesh : (comp.numMaterials - 1)];
//
//				DrawParams* params = MemoryAllocator::Allocate<DrawParams>(allocNo);
//				params->pipelineState = materialPSOs[kMaterialTypeDepth];
//				params->vertexBuffer = mesh.VertexBuffer;
//				params->indexBuffer = mesh.IndexBuffer;
//				params->startIndex = mesh.StartIndex;
//				params->startVertex = mesh.StartVertex;
//				params->indexCount = mesh.NumIndices;
//				params->renderTargets[0] = TextureHandle();
//				params->depthRenderTarget = lights[0].depthMap;
//
//
//				params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(i * 16), 16 };
//				params->vsConstantBuffers[1] = { shadowDepthBuffer, 0, 16 };
//				//params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 16 };
//
//				cmdBuf->Add(RendererCommand::kCommandDraw, params);
//			}
//		}
//
//		for (uint32_t i = numActiveRenderables - 1; i < numActiveRenderables; ++i)
//		{
//			RenderingComponentData& comp = renderables[activeRenderables[i]];
//
//			assert(nullptr != comp.model);
//			assert(0 != comp.numMaterials);
//
//			Model& model = *comp.model;
//			//Material* mat = comp.material;
//
//			for (uint32_t iMesh = 0; iMesh < model.numMeshes; ++iMesh)
//			{
//				assert(model.meshes[iMesh]);
//				Mesh& mesh = meshes[model.meshes[iMesh].id];
//				Material* mat = comp.materials[iMesh < comp.numMaterials ? iMesh : (comp.numMaterials - 1)];
//
//				DrawParams* params = MemoryAllocator::Allocate<DrawParams>(allocNo);
//				params->pipelineState = materialPSOs[kMaterialTypeOpaqueSkinned];
//				params->vertexBuffer = mesh.VertexBuffer;
//				params->indexBuffer = mesh.IndexBuffer;
//				params->startIndex = mesh.StartIndex;
//				params->startVertex = mesh.StartVertex;
//				params->indexCount = mesh.NumIndices;
//				params->renderTargets[0] = TextureHandle();
//				params->depthRenderTarget = lights[0].depthMap;
//
//				params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(i * 16), 16 };
//				params->vsConstantBuffers[1] = { shadowDepthBuffer, 0, 16 };
//				//params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 16 };
//
//				(void*)0;
//				{
//					AnimationComponent anim = comp.entity.GetComponent<AnimationComponent>();
//					if (!anim || !anim->boneMatricesBuffer)
//					{
//						return kErrUnknown;
//					}
//					params->vsConstantBuffers[2] = { anim->boneMatricesBuffer, 0, 0 };
//				}
//
//				cmdBuf->Add(RendererCommand::kCommandDraw, params);
//			}
//		}


		// generate draw call for active renderables in command buffer
		for (uint32_t i = 0; i < numActiveRenderables; ++i)
			//	for (uint32_t i = 0; i < 1; ++i)
		{
			RenderingComponentData& comp = renderables[activeRenderables[i]];

			assert(nullptr != comp.model);
			assert(0 != comp.numMaterials);

			Model& model = *comp.model;
			//Material* mat = comp.material;

			for (uint32_t iMesh = 0; iMesh < model.numMeshes; ++iMesh)
			{
				assert(model.meshes[iMesh]);
				Mesh& mesh = meshes[model.meshes[iMesh].id];
				Material* mat = comp.materials[iMesh < comp.numMaterials ? iMesh : (comp.numMaterials - 1)];

				DrawParams* params = MemoryAllocator::Allocate<DrawParams>(allocNo);
				params->pipelineState = materialPSOs[mat->type];
				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				if (mat->type == kMaterialTypeOpaque) params->pipelineState = materialPSOs[kMaterialDeferredGeometryOpaque];
				if (mat->type == kMaterialTypeOpaqueSkinned) params->pipelineState = materialPSOs[kMaterialDeferredGeometryOpaqueSkinned];

				switch (mat->type)
				{
				case kMaterialTypeTest:
					params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(i * 16), 16 };
					params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 16 };
					params->psTextures[0] = mat->mainTex;
					params->psSamplers[0] = defaultSampler;
					break;
				case kMaterialTypeOpaqueSkinned:
					(void*)0;
					{
						AnimationComponent anim = comp.entity.GetComponent<AnimationComponent>();
						if (!anim || !anim->boneMatricesBuffer)
						{
							return kErrUnknown;
						}
						params->vsConstantBuffers[2] = { anim->boneMatricesBuffer, 0, 0 };
					}
					// fall through
				case kMaterialTypeOpaque:
					params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(i * 16), 16 };
					params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 16 };
					params->psTextures[0] = mat->mainTex;
					params->psTextures[1] = mat->normalMap;
					params->psSamplers[0] = defaultSampler;

					params->renderTargets[0] = gBufferPosition;
					params->renderTargets[1] = gBufferNormal;
					params->renderTargets[2] = gBufferAlbedo;
					params->depthRenderTarget = gBufferDepth;

					break;
				default:
					assert(false && "this material type is not applicable for entities");
					break;
				}

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}
		}

		return kOK;
	}
}
