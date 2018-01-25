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
		tofu::math::float4	    lightColor;			// 1 shader constants
		tofu::math::float3	    lightDirection;	    // 1 shader constants
		float					padding1;		
		tofu::math::float3		lightPosition;		// 1 shader constants
		float					padding2;
		tofu::math::float3		cameraPos;			// 1 shader constants
		float					padding7[4 * 12];	// 12 shader constants
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
		CHECKED(InitBuiltinShader(kMaterialTypeTest,
			"assets/test_vs.shader",
			"assets/test_ps.shader"
		));

		CHECKED(InitBuiltinShader(kMaterialTypeSkybox,
			"assets/skybox_vs.shader",
			"assets/skybox_ps.shader"
		));

		CHECKED(InitBuiltinShader(kMaterialTypeOpaque,
			"assets/opaque_vs.shader",
			"assets/opaque_ps.shader"
		));

		CHECKED(InitBuiltinShader(kMaterialTypeOpaqueSkinned,
			"assets/opaque_skinned_vs.shader",
			"assets/opaque_ps.shader"
		));

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

				cmdBuf->Add(RendererCommand::kCommandCreateSampler, params);
			}
		}

		builtinCube = CreateModel("assets/cube.model");
		assert(nullptr != builtinCube);

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
		LightComponentData& light0 = LightComponent::GetAllComponents()[0];

		math::float3 camPos;
		{
			FrameConstants* data = reinterpret_cast<FrameConstants*>(
				MemoryAllocator::Allocators[allocNo].Allocate(sizeof(FrameConstants), 4)
				);
			assert(nullptr != data);
			data->matView = camera.CalcViewMatrix();
			data->matProj = camera.CalcProjectionMatrix();

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

		// Light constant buffer data
		
		{
			LightingConstants* data = reinterpret_cast<LightingConstants*>(
				MemoryAllocator::Allocators[allocNo].Allocate(sizeof(LightingConstants), 4)
				);
			assert(nullptr != data);
			data->lightColor = light0.lightColor;

			TransformComponent t = light0.entity.GetComponent<TransformComponent>();
			data->lightDirection = math::float3{ (t->GetWorldRotation() * t->GetRightVector()).x,
				(t->GetWorldRotation() * t->GetUpVector()).y,
				(t->GetWorldRotation() * t->GetForwardVector()).z };
			data->lightPosition = t->GetWorldPosition();

			data->cameraPos = camPos;

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
		{
			RenderingComponentData& comp = renderables[i];
			TransformComponent transform = comp.entity.GetComponent<TransformComponent>();
			assert(transform);

			// TODO culling

			uint32_t idx = numActiveRenderables++;
			activeRenderables[idx] = i;
			transformArray[idx * 4] = transform->GetWorldTransform().GetMatrix();
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

		// generate draw call for active renderables in command buffer
		for (uint32_t i = 0; i < numActiveRenderables; ++i)
		{
			RenderingComponentData& comp = renderables[activeRenderables[i]];

			assert(nullptr != comp.model && nullptr != comp.material);

			Model& model = *comp.model;
			Material* mat = comp.material;
			

			for (uint32_t iMesh = 0; iMesh < model.numMeshes; ++iMesh)
			{
				assert(model.meshes[iMesh]);
				Mesh& mesh = meshes[model.meshes[iMesh].id];

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
					// fall through
				case kMaterialTypeOpaque:
					params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(i * 16), 16 };
					params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 16 };
					params->psConstantBuffers[0] = { lightingConstantBuffer,0,16 };
					params->psTextures[0] = skyboxTex;
					params->psTextures[1] = mat->mainTex;
					params->psTextures[2] = mat->normalMap;
					params->psSamplers[0] = defaultSampler;
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
				return &models[iter->second];
			}
		}

		uint8_t* data = nullptr;
		size_t size = 0u;
		int32_t err = FileIO::ReadFile(filename, reinterpret_cast<void**>(&data), &size, 4, kAllocFrameBasedMem);
		if (kOK != err)
		{
			return nullptr;
		}

		// read header
		model::ModelHeader* header = reinterpret_cast<model::ModelHeader*>(data);

		assert(header->Magic == model::kModelFileMagic);
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

		// keep pointers to bone and animation structures
		if (header->NumBones > 0)
		{
			model.bones = reinterpret_cast<model::ModelBone*>(indices + indexBufferSize);
			if (header->HasAnimation)
			{
				model.animations = reinterpret_cast<model::ModelAnimation*>(
					model.bones + header->NumBones
					);

				model.channels = reinterpret_cast<model::ModelAnimChannel*>(
					model.animations + header->NumAnimations
					);

				model.translationFrames = reinterpret_cast<model::ModelFloat3Frame*>(
					model.channels + header->NumAnimChannels
					);

				model.rotationFrames = reinterpret_cast<model::ModelQuatFrame*>(
					model.translationFrames + header->NumTotalTranslationFrames
					);

				model.scaleFrames = reinterpret_cast<model::ModelFloat3Frame*>(
					model.rotationFrames + header->NumTotalRotationFrames
					);

				model.frames = reinterpret_cast<model::ModelAnimFrame*>(
					model.scaleFrames + header->NumTotalScaleFrames
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

		return &model;
	}

	TextureHandle RenderingSystem::CreateTexture(const char* filename)
	{
		void* data = nullptr;
		size_t size = 0;
		int32_t err = FileIO::ReadFile(filename, &data, &size, 4, allocNo);

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

	int32_t RenderingSystem::InitBuiltinShader(MaterialType matType, const char * vsFile, const char * psFile)
	{
		if (materialVSs[matType] || materialPSs[matType])
			return kErrUnknown;
		
		materialVSs[matType] = vertexShaderHandleAlloc.Allocate();
		materialPSs[matType] = pixelShaderHandleAlloc.Allocate();
		if (!materialVSs[matType] || !materialPSs[matType])
		{
			return kErrUnknown;
		}

		{
			CreateVertexShaderParams* params = MemoryAllocator::Allocate<CreateVertexShaderParams>(allocNo);
			assert(nullptr != params);
			params->handle = materialVSs[matType];

			int32_t err = FileIO::ReadFile(
				vsFile,
				&(params->data),
				&(params->size),
				4,
				allocNo);

			if (kOK != err)
			{
				return err;
			}

			cmdBuf->Add(RendererCommand::kCommandCreateVertexShader, params);
		}

		{
			CreatePixelShaderParams* params = MemoryAllocator::Allocate<CreatePixelShaderParams>(allocNo);
			assert(nullptr != params);
			params->handle = materialPSs[matType];

			int32_t err = FileIO::ReadFile(
				psFile,
				&(params->data),
				&(params->size),
				4,
				allocNo);

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

		if (c.caches) {
			// TODO dealloc
			free(c.caches);
		}

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

		// TODO: buffer
		c.caches = (AnimationFrameCache*)malloc(c.model->header->NumBones * sizeof(AnimationFrameCache));
		c.ResetCaches();

		return kOK;
	}
}
