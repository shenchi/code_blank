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
		assert(TF_OK == MemoryAllocator::Allocators[ALLOC_LEVEL_BASED_MEM].Init(
			LEVEL_BASED_MEM_SIZE,
			LEVEL_BASED_MEM_ALIGN)
		);

		for (uint32_t i = ALLOC_FRAME_BASED_MEM;
			i <= ALLOC_FRAME_BASED_MEM_END;
			++i)
		{
			assert(TF_OK == MemoryAllocator::Allocators[i].Init(
				FRAME_BASED_MEM_SIZE,
				FRAME_BASED_MEM_ALIGN)
			);
		}

		// Initialize Renderer Backend
		assert(TF_OK == renderer->Init());

		//
		frameNo = 0;
		BeginFrame();

		// Create Built-in Shaders
		assert(TF_OK == InitBuiltinShader(TestMaterial,
			"assets/test_vs.shader",
			"assets/test_ps.shader"
		));

		assert(TF_OK == InitBuiltinShader(SkyboxMaterial,
			"assets/skybox_vs.shader",
			"assets/skybox_ps.shader"
		));

		assert(TF_OK == InitBuiltinShader(OpaqueMaterial,
			"assets/opaque_vs.shader",
			"assets/opaque_ps.shader"
		));

		assert(TF_OK == InitBuiltinShader(OpaqueSkinnedMaterial,
			"assets/opaque_skinned_vs.shader",
			"assets/opaque_ps.shader"
		));

		// constant buffers
		{
			transformBuffer = bufferHandleAlloc.Allocate();
			assert(transformBuffer);

			transformBufferSize = sizeof(math::float4x4) * 4 * MAX_ENTITIES;

			{
				CreateBufferParams* params = MemoryAllocator::Allocate<CreateBufferParams>(allocNo);
				assert(nullptr != params);
				params->handle = transformBuffer;
				params->bindingFlags = BINDING_CONSTANT_BUFFER;
				params->size = transformBufferSize;
				params->dynamic = 1;

				cmdBuf->Add(RendererCommand::CreateBuffer, params);
			}

			frameConstantBuffer = bufferHandleAlloc.Allocate();
			assert(frameConstantBuffer);

			{
				CreateBufferParams* params = MemoryAllocator::Allocate<CreateBufferParams>(allocNo);
				assert(nullptr != params);
				params->handle = frameConstantBuffer;
				params->bindingFlags = BINDING_CONSTANT_BUFFER;
				params->size = sizeof(FrameConstants);
				params->dynamic = 1;

				cmdBuf->Add(RendererCommand::CreateBuffer, params);
			}
		}

		// create built-in pipeline states
		{
			materialPSOs[TestMaterial] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[TestMaterial]);

			{
				CreatePipelineStateParams* params = MemoryAllocator::Allocate<CreatePipelineStateParams>(allocNo);
				params->handle = materialPSOs[TestMaterial];
				params->vertexShader = materialVSs[TestMaterial];
				params->pixelShader = materialPSs[TestMaterial];

				cmdBuf->Add(RendererCommand::CreatePipelineState, params);
			}

			materialPSOs[SkyboxMaterial] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[SkyboxMaterial]);

			{
				CreatePipelineStateParams* params = MemoryAllocator::Allocate<CreatePipelineStateParams>(allocNo);
				params->handle = materialPSOs[SkyboxMaterial];
				params->vertexShader = materialVSs[SkyboxMaterial];
				params->pixelShader = materialPSs[SkyboxMaterial];
				params->CullMode = CULL_FRONT;
				params->DepthFunc = COMPARISON_ALWAYS;
				cmdBuf->Add(RendererCommand::CreatePipelineState, params);
			}

			materialPSOs[OpaqueMaterial] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[OpaqueMaterial]);

			{
				CreatePipelineStateParams* params = MemoryAllocator::Allocate<CreatePipelineStateParams>(allocNo);
				params->handle = materialPSOs[OpaqueMaterial];
				params->vertexShader = materialVSs[OpaqueMaterial];
				params->pixelShader = materialPSs[OpaqueMaterial];

				cmdBuf->Add(RendererCommand::CreatePipelineState, params);
			}

			materialPSOs[OpaqueSkinnedMaterial] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[OpaqueSkinnedMaterial]);

			{
				CreatePipelineStateParams* params = MemoryAllocator::Allocate<CreatePipelineStateParams>(allocNo);
				params->handle = materialPSOs[OpaqueSkinnedMaterial];
				params->vertexShader = materialVSs[OpaqueSkinnedMaterial];
				params->pixelShader = materialPSs[OpaqueSkinnedMaterial];
				params->vertexFormat = VERTEX_FORMAT_SKINNED;

				cmdBuf->Add(RendererCommand::CreatePipelineState, params);
			}
		}

		// create default sampler
		{
			defaultSampler = samplerHandleAlloc.Allocate();
			assert(defaultSampler);

			{
				CreateSamplerParams* params = MemoryAllocator::Allocate<CreateSamplerParams>(allocNo);
				params->handle = defaultSampler;

				cmdBuf->Add(RendererCommand::CreateSampler, params);
			}
		}

		builtinCube = CreateModel("assets/cube.model");
		assert(nullptr != builtinCube);

		return TF_OK;
	}

	int32_t RenderingSystem::Shutdown()
	{
		renderer->Release();

		for (uint32_t i = ALLOC_FRAME_BASED_MEM;
			i <= ALLOC_FRAME_BASED_MEM_END;
			++i)
		{
			assert(TF_OK == MemoryAllocator::Allocators[i].Shutdown());
		}

		assert(TF_OK == MemoryAllocator::Allocators[ALLOC_LEVEL_BASED_MEM].Shutdown());
		return TF_OK;
	}

	int32_t RenderingSystem::BeginFrame()
	{
		if (nullptr != cmdBuf)
		{
			return TF_OK;
		}

		//assert(false && "sync need to be done.");

		allocNo = ALLOC_FRAME_BASED_MEM + frameNo % FRAME_BUFFER_COUNT;
		MemoryAllocator::Allocators[allocNo].Reset();

		cmdBuf = RendererCommandBuffer::Create(COMMAND_BUFFER_CAPACITY, allocNo);
		assert(nullptr != cmdBuf);

		return TF_OK;
	}

	int32_t RenderingSystem::Update()
	{
		assert(CameraComponent::GetNumComponents() > 0);
		CameraComponentData& camera = CameraComponent::GetAllComponents()[0];

		// update aspect
		{
			int32_t w, h;
			renderer->GetFrameBufferSize(w, h);
			camera.SetAspect(h == 0 ? 1.0f : float(w) / h);
		}

		{
			FrameConstants* data = reinterpret_cast<FrameConstants*>(
				MemoryAllocator::Allocators[allocNo].Allocate(sizeof(FrameConstants), 4)
				);
			assert(nullptr != data);
			data->matView = camera.CalcViewMatrix();
			data->matProj = camera.CalcProjectionMatrix();

			TransformComponent t = camera.entity.GetComponent<TransformComponent>();
			data->cameraPos = t->GetWorldPosition();

			UpdateBufferParams* params = MemoryAllocator::Allocate<UpdateBufferParams>(allocNo);
			assert(nullptr != params);
			params->handle = frameConstantBuffer;
			params->data = data;
			params->size = sizeof(FrameConstants);

			cmdBuf->Add(RendererCommand::UpdateBuffer, params);
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

			cmdBuf->Add(RendererCommand::ClearRenderTargets, params);
		}
		else
		{
			assert(camera.skybox->type == SkyboxMaterial);
			skyboxTex = camera.skybox->mainTex;

			Mesh& mesh = meshes[builtinCube->meshes[0].id];

			DrawParams* params = MemoryAllocator::Allocate<DrawParams>(allocNo);
			params->pipelineState = materialPSOs[SkyboxMaterial];
			params->vertexBuffer = mesh.VertexBuffer;
			params->indexBuffer = mesh.IndexBuffer;
			params->startIndex = mesh.StartIndex;
			params->startVertex = mesh.StartVertex;
			params->indexCount = mesh.NumIndices;
			params->vsConstantBuffers[0] = { frameConstantBuffer, 0, 16 };

			params->psTextures[0] = skyboxTex;
			params->psSamplers[0] = defaultSampler;

			cmdBuf->Add(RendererCommand::Draw, params);
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
			MemoryAllocator::Allocators[allocNo].Allocate(sizeof(uint32_t) * MAX_ENTITIES, 4)
			);

		assert(nullptr != transformArray && nullptr != activeRenderables);

		uint32_t numActiveRenderables = 0;

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

		if (numActiveRenderables > 0)
		{
			UpdateBufferParams* params = MemoryAllocator::Allocate<UpdateBufferParams>(allocNo);
			assert(nullptr != params);
			params->handle = transformBuffer;
			params->data = transformArray;
			params->size = sizeof(math::float4x4) * 4 * numActiveRenderables;

			cmdBuf->Add(RendererCommand::UpdateBuffer, params);
		}

		AnimationComponentData* animComps = AnimationComponent::GetAllComponents();
		uint32_t animCompCount = AnimationComponent::GetNumComponents();

		for (uint32_t i = 0; i < animCompCount; i++)
		{
			AnimationComponentData& anim = animComps[i];
			RenderingComponent r = anim.entity.GetComponent<RenderingComponent>();

			if (!r || nullptr == r->model || !r->model->HasAnimation())
			{
				return TF_UNKNOWN_ERR;
			}

			if (anim.model != r->model)
			{
				anim.model = r->model;
				int32_t err = ReallocAnimationResources(anim);
				if (TF_OK != err)
				{
					return err;
				}
			}

			void* boneMatrices = MemoryAllocator::Allocators[allocNo].Allocate(anim.boneMatricesBufferSize, 4);

			anim.UpdateTiming();
			anim.FillInBoneMatrices(boneMatrices, anim.boneMatricesBufferSize);

			UpdateBufferParams* params = MemoryAllocator::Allocate<UpdateBufferParams>(allocNo);
			assert(nullptr != params);
			params->handle = anim.boneMatricesBuffer;
			params->data = boneMatrices;
			params->size = anim.boneMatricesBufferSize;

			cmdBuf->Add(RendererCommand::UpdateBuffer, params);
		}

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
				case TestMaterial:
					params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(i * 16), 16 };
					params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 16 };
					params->psTextures[0] = mat->mainTex;
					params->psSamplers[0] = defaultSampler;
					break;
				case OpaqueSkinnedMaterial:
					(void*)0;
					{
						AnimationComponent anim = comp.entity.GetComponent<AnimationComponent>();
						if (!anim || !anim->boneMatricesBuffer)
						{
							return TF_UNKNOWN_ERR;
						}
						params->vsConstantBuffers[2] = { anim->boneMatricesBuffer, 0, 0 };
					}
					// fall through
				case OpaqueMaterial:
					params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(i * 16), 16 };
					params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 16 };
					params->psConstantBuffers[0] = { frameConstantBuffer, 16, 16 };
					params->psTextures[0] = skyboxTex;
					params->psTextures[1] = mat->mainTex;
					params->psTextures[2] = mat->normalMap;
					params->psSamplers[0] = defaultSampler;
					break;
				default:
					assert(false && "this material type is not applicable for entities");
					break;
				}

				cmdBuf->Add(RendererCommand::Draw, params);
			}
		}

		return TF_OK;
	}

	int32_t RenderingSystem::EndFrame()
	{
		assert(TF_OK == renderer->Submit(cmdBuf));

		renderer->Present();

		frameNo++;

		cmdBuf = nullptr;

		return TF_OK;
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
		int32_t err = FileIO::ReadFile(filename, reinterpret_cast<void**>(&data), &size, 4, ALLOC_FRAME_BASED_MEM);
		if (TF_OK != err)
		{
			return nullptr;
		}

		model::ModelHeader* header = reinterpret_cast<model::ModelHeader*>(data);

		assert(header->Magic == model::MODEL_FILE_MAGIC);
		assert(header->StructOfArray == 0);
		assert(header->HasIndices == 1);
		assert(header->HasTangent == 1);
		assert(header->NumTexcoordChannels == 1);

		if (header->NumMeshes == 0)
		{
			return nullptr;
		}

		model::ModelMesh* meshInfos = reinterpret_cast<model::ModelMesh*>(header + 1);

		uint32_t verticesCount = 0;
		uint32_t indicesCount = 0;

		assert(header->NumMeshes <= MAX_MESHES_PER_MODEL);
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

		BufferHandle vbHandle = bufferHandleAlloc.Allocate();
		BufferHandle ibHandle = bufferHandleAlloc.Allocate();
		assert(vbHandle && ibHandle);

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

		if (indicesCount % 2 != 0)
		{
			indicesCount += 1;
		}

		uint32_t vertexBufferSize = verticesCount * header->CalculateVertexSize();
		uint32_t indexBufferSize = indicesCount * sizeof(uint16_t);

		uint8_t* vertices = reinterpret_cast<uint8_t*>(meshInfos + header->NumMeshes);
		uint8_t* indices = vertices + vertexBufferSize;

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
			}
		}

		{
			CreateBufferParams* params = MemoryAllocator::Allocate<CreateBufferParams>(ALLOC_FRAME_BASED_MEM);
			params->handle = vbHandle;
			params->bindingFlags = BINDING_VERTEX_BUFFER;
			params->data = vertices;
			params->size = vertexBufferSize;
			params->stride = header->CalculateVertexSize();

			cmdBuf->Add(RendererCommand::CreateBuffer, params);
		}

		{
			CreateBufferParams* params = MemoryAllocator::Allocate<CreateBufferParams>(ALLOC_FRAME_BASED_MEM);
			params->handle = ibHandle;
			params->bindingFlags = BINDING_INDEX_BUFFER;
			params->data = indices;
			params->size = indexBufferSize;

			cmdBuf->Add(RendererCommand::CreateBuffer, params);
		}

		return &model;
	}

	TextureHandle RenderingSystem::CreateTexture(const char* filename)
	{
		void* data = nullptr;
		size_t size = 0;
		int32_t err = FileIO::ReadFile(filename, &data, &size, 4, allocNo);

		if (TF_OK != err)
		{
			return TextureHandle();
		}

		TextureHandle handle = textureHandleAlloc.Allocate();
		assert(handle);

		CreateTextureParams* params = MemoryAllocator::Allocate<CreateTextureParams>(allocNo);

		params->handle = handle;
		params->bindingFlags = BINDING_SHADER_RESOURCE;
		params->isFile = 1;
		params->data = data;
		params->width = static_cast<uint32_t>(size);

		cmdBuf->Add(RendererCommand::CreateTexture, params);

		return handle;
	}

	Material* RenderingSystem::CreateMaterial(MaterialType type)
	{
		MaterialHandle handle = materialHandleAlloc.Allocate();
		assert(handle);

		Material* mat = &(materials[handle.id]);
		new (mat) Material(type);
		mat->handle = handle;

		return mat;
	}

	int32_t RenderingSystem::InitBuiltinShader(MaterialType matType, const char * vsFile, const char * psFile)
	{
		if (materialVSs[matType] || materialPSs[matType])
			return TF_UNKNOWN_ERR;
		
		materialVSs[matType] = vertexShaderHandleAlloc.Allocate();
		materialPSs[matType] = pixelShaderHandleAlloc.Allocate();
		assert(materialVSs[matType] && materialPSs[matType]);

		{
			CreateVertexShaderParams* params = MemoryAllocator::Allocate<CreateVertexShaderParams>(allocNo);
			assert(nullptr != params);
			params->handle = materialVSs[matType];

			assert(TF_OK == FileIO::ReadFile(
				vsFile,
				&(params->data),
				&(params->size),
				4,
				allocNo)
			);

			cmdBuf->Add(RendererCommand::CreateVertexShader, params);
		}

		{
			CreatePixelShaderParams* params = MemoryAllocator::Allocate<CreatePixelShaderParams>(allocNo);
			assert(nullptr != params);
			params->handle = materialPSs[matType];

			assert(TF_OK == FileIO::ReadFile(
				psFile,
				&(params->data),
				&(params->size),
				4,
				allocNo)
			);

			cmdBuf->Add(RendererCommand::CreatePixelShader, params);
		}
		return TF_OK;
	}

	int32_t RenderingSystem::ReallocAnimationResources(AnimationComponentData & c)
	{
		if (c.boneMatricesBuffer)
		{
			// TODO dealloc
		}

		Model* model = c.model;
		if (nullptr == model || nullptr == model->header || !model->HasAnimation() || 0 == model->header->NumBones)
		{
			return TF_UNKNOWN_ERR;
		}

		BufferHandle bufferHandle = bufferHandleAlloc.Allocate();
		if (!bufferHandle)
		{
			return TF_UNKNOWN_ERR;
		}

		c.boneMatricesBuffer = bufferHandle;
		c.boneMatricesBufferSize = static_cast<uint32_t>(sizeof(math::float4x4)) * model->header->NumBones;

		CreateBufferParams* params = MemoryAllocator::Allocate<CreateBufferParams>(allocNo);
		assert(nullptr != params);
		params->handle = bufferHandle;
		params->bindingFlags = BINDING_CONSTANT_BUFFER;
		params->size = c.boneMatricesBufferSize;
		params->dynamic = 1;

		cmdBuf->Add(RendererCommand::CreateBuffer, params);

		return TF_OK;
	}


}
