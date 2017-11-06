#include "RenderingSystem.h"

#include <cassert>

#include "Renderer.h"

#include "MemoryAllocator.h"
#include "FileIO.h"

#include "ModelFormat.h"

#include "TransformComponent.h"
#include "CameraComponent.h"
#include "RenderingComponent.h"

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
		vertexShaderHandleAlloc(),
		pixelShaderHandleAlloc(),
		pipelineStateHandleAlloc(),
		frameNo(0),
		allocNo(0),
		defaultVertexShader(),
		defaultPixelShader(),
		meshes(),
		models(),
		materials(),
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
		{
			defaultVertexShader = vertexShaderHandleAlloc.Allocate();
			defaultPixelShader = pixelShaderHandleAlloc.Allocate();
			assert(defaultVertexShader && defaultPixelShader);

			{
				CreateVertexShaderParams* params = MemoryAllocator::Allocate<CreateVertexShaderParams>(allocNo);
				assert(nullptr != params);
				params->handle = defaultVertexShader;

				assert(TF_OK == FileIO::ReadFile(
					"assets/test_vs.shader",
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
				params->handle = defaultPixelShader;

				assert(TF_OK == FileIO::ReadFile(
					"assets/test_ps.shader",
					&(params->data),
					&(params->size),
					4,
					allocNo)
				);

				cmdBuf->Add(RendererCommand::CreatePixelShader, params);
			}
		}

		// constant buffers
		{
			transformBuffer = bufferHandleAlloc.Allocate();
			assert(transformBuffer);

			transformBufferSize = sizeof(math::float4x4) * MAX_ENTITIES;

			{
				CreateBufferParams* params = MemoryAllocator::Allocate<CreateBufferParams>(allocNo);
				assert(nullptr != params);
				params->handle = transformBuffer;
				params->bindingFlags = BINDING_CONSTANT_BUFFER;
				params->dynamic = 1;
				params->size = transformBufferSize;

				cmdBuf->Add(RendererCommand::CreateBuffer, params);
			}

			frameConstantBuffer = bufferHandleAlloc.Allocate();
			assert(frameConstantBuffer);

			frameConstantSize = sizeof(math::float4x4) * 2;

			{
				CreateBufferParams* params = MemoryAllocator::Allocate<CreateBufferParams>(allocNo);
				assert(nullptr != params);
				params->handle = frameConstantBuffer;
				params->bindingFlags = BINDING_CONSTANT_BUFFER;
				params->dynamic = 1;
				params->size = frameConstantSize;

				cmdBuf->Add(RendererCommand::CreateBuffer, params);
			}
		}

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

		{
			math::float4x4* data = reinterpret_cast<math::float4x4*>(MemoryAllocator::Allocators[allocNo].Allocate(sizeof(math::float4x4) * 2, 4));
			assert(nullptr != data);
			*data = camera.CalcViewMatrix();
			*(data + 1) = camera.CalcProjectionMatrix();

			UpdateBufferParams* params = MemoryAllocator::Allocate<UpdateBufferParams>(allocNo);
			assert(nullptr != params);
			params->handle = frameConstantBuffer;
			params->data = data;
			params->size = frameConstantSize;

			cmdBuf->Add(RendererCommand::UpdateBuffer, params);
		}

		{	// clear
			ClearParams* params = MemoryAllocator::Allocate<ClearParams>(allocNo);

			const math::float4& clearColor = camera.GetClearColor();

			params->clearColor[0] = clearColor.x;
			params->clearColor[1] = clearColor.y;
			params->clearColor[2] = clearColor.z;
			params->clearColor[3] = clearColor.w;

			cmdBuf->Add(RendererCommand::ClearRenderTargets, params);
		}

		// get all renderables in system
		RenderingComponentData* comps = RenderingComponent::GetAllComponents();
		uint32_t count = RenderingComponent::GetNumComponents();

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

		for (uint32_t i = 0; i < count; ++i)
		{
			RenderingComponentData& comp = comps[i];
			TransformComponent transform = comp.entity.GetComponent<TransformComponent>();
			assert(transform);

			// TODO culling

			uint32_t idx = numActiveRenderables++;
			activeRenderables[idx] = i;
			transformArray[idx] = transform->GetWorldTransform().GetMatrix();
		}

		{
			UpdateBufferParams* params = MemoryAllocator::Allocate<UpdateBufferParams>(allocNo);
			assert(nullptr != params);
			params->handle = transformBuffer;
			params->data = transformArray;
			params->size = sizeof(math::float4x4) * numActiveRenderables;

			cmdBuf->Add(RendererCommand::UpdateBuffer, params);
		}

		for (uint32_t i = 0; i < numActiveRenderables; ++i)
		{
			RenderingComponentData& comp = comps[activeRenderables[i]];

			assert(comp.model && comp.material);

			Model& model = models[comp.model.id];
			Material& mat = materials[comp.material.id];

			for (uint32_t iMesh = 0; iMesh < model.NumMeshes; ++iMesh)
			{
				assert(model.Meshes[iMesh]);
				Mesh& mesh = meshes[model.Meshes[iMesh].id];

				DrawParams* params = MemoryAllocator::Allocate<DrawParams>(allocNo);
				params->pipelineState = mat.pipelineState;
				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;
				params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(i * 4), 4 };
				params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 0 };

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

	ModelHandle RenderingSystem::CreateModel(const char* filename)
	{
		std::string strFilename(filename);

		{
			auto iter = modelTable.find(strFilename);
			if (iter != modelTable.end())
			{
				return iter->second;
			}
		}

		uint8_t* data = nullptr;
		size_t size = 0u;
		int32_t err = FileIO::ReadFile(filename, reinterpret_cast<void**>(&data), &size, 4, ALLOC_FRAME_BASED_MEM);
		if (TF_OK != err)
		{
			return ModelHandle();
		}

		model::ModelHeader* header = reinterpret_cast<model::ModelHeader*>(data);

		assert(header->Magic == model::MODEL_FILE_MAGIC);
		assert(header->StructOfArray == 0);
		assert(header->HasIndices == 1);
		assert(header->HasTangent == 1);
		assert(header->NumTexcoordChannels == 1);

		if (header->NumMeshes == 0)
		{
			return ModelHandle();
		}

		model::ModelMesh* meshInfos = reinterpret_cast<model::ModelMesh*>(header + 1);

		uint32_t verticesCount = 0;
		uint32_t indicesCount = 0;

		assert(header->NumMeshes <= MAX_MESHES_PER_MODEL);
		ModelHandle modelHandle = modelHandleAlloc.Allocate();
		assert(modelHandle);
		Model& model = models[modelHandle.id];

		model = {};
		model.NumMeshes = header->NumMeshes;

		BufferHandle vbHandle = bufferHandleAlloc.Allocate();
		BufferHandle ibHandle = bufferHandleAlloc.Allocate();
		assert(vbHandle && ibHandle);

		for (uint32_t i = 0; i < header->NumMeshes; ++i)
		{
			model.Meshes[i] = meshHandleAlloc.Allocate();
			assert(model.Meshes[i]);
			uint32_t id = model.Meshes[i].id;
			meshes[id] = {};
			meshes[id].VertexBuffer = vbHandle;
			meshes[id].IndexBuffer = ibHandle;
			meshes[id].StartVertex = verticesCount;
			meshes[id].StartIndex = indicesCount;
			meshes[id].NumVertices = meshInfos[i].NumVertices;
			meshes[id].NumIndices = meshInfos[i].NumIndices;

			verticesCount += meshInfos[i].NumVertices;
			indicesCount += meshInfos[i].NumIndices;
		}

		uint32_t vertexBufferSize = verticesCount * header->CalculateVertexSize();
		uint32_t indexBufferSize = indicesCount * sizeof(uint16_t);

		uint8_t* vertices = reinterpret_cast<uint8_t*>(meshInfos + header->NumMeshes);
		uint8_t* indices = vertices + vertexBufferSize;

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

		return modelHandle;
	}

	MaterialHandle RenderingSystem::CreateMaterial(MaterialType type)
	{
		MaterialHandle handle = materialHandleAlloc.Allocate();
		assert(handle);

		PipelineStateHandle psHandle = pipelineStateHandleAlloc.Allocate();
		assert(psHandle);

		{
			CreatePipelineStateParams* params = MemoryAllocator::Allocate<CreatePipelineStateParams>(allocNo);
			params->handle = psHandle;
			params->vertexShader = defaultVertexShader;
			params->pixelShader = defaultPixelShader;

			cmdBuf->Add(RendererCommand::CreatePipelineState, params);
		}

		Material& mat = materials[handle.id];
		mat.pipelineState = psHandle;

		return handle;
	}

}
