#include "RenderingSystem.h"

#include <cassert>
#include <algorithm>

#include "Engine.h"

#include "Renderer.h"

#include "MemoryAllocator.h"
#include "FileIO.h"

#include "ModelFormat.h"

#include "TransformComponent.h"
#include "CameraComponent.h"
#include "RenderingComponent.h"
#include "AnimationComponent.h"
#include "LightComponent.h"

#include "Collision.h"

#define TOFU_FILTER_VFOG 1

namespace
{
	using tofu::math::float4x4;
	using tofu::math::float4;
	using tofu::math::float3;

	struct FrameConstants							// 32 shader constants in total
	{
		float4x4				matView;			// 4 shader constants
		float4x4				matProj;			// 4 shader constants
		float4x4				matViewInv;			// 4 shader constants
		float4x4				matProjInv;			// 4 shader constants
		float4					cameraPos;			// 1 shader constants
		float4					bufferSize;			// 1 shader constants
		float4					leftTopRay;			// 1 shader constants
		float4					rightTopRay;		// 1 shader constants
		float4					leftBottomRay;		// 1 shader constants
		float4					rightBottomRay;		// 1 shader constants
		float4					perspectiveParams;	// 1 shader constants (fov, aspect, zNear, zFar)
		float					padding3[4 * 9];	// 9 shader constants
	};

	struct DirectionalLight
	{
		float4					direction;
		float4					color;
		float					intensity;
		float					_padding1;
		float					_padding2;
		float					_padding3;
	};

	struct PointLight
	{
		float4					color;
		float					intensity;
		float					range;
		float					_padding1;
		float					_padding2;
	};

	struct SpotLight
	{
		float4					direction;
		float4					color;
		float					intensity;
		float					range;
		float					spotAngle;
		uint32_t				shadowId;
	};

	struct PointLightParams
	{
		PointLight				lights[tofu::kMaxPointLights];
		uint32_t				numLights;
		float					padding[3];
	};

	struct SpotLightParams
	{
		SpotLight				lights[tofu::kMaxSpotLights];
		uint32_t				numLights;
		float					padding[3];
	};

	struct LightParametersDirectionalAndAmbient
	{
		DirectionalLight		directionalLights[tofu::kMaxDirectionalLights];
		float4					ambient;
	};

	struct VolumetricFogParams
	{
		float4					fogParams;
		float3					windDir;
		float					time;
		float					noiseScale;
		float					noiseBase;
		float					noiseAmp;
		float					density;
		float					maxFarClip;
		float					maxZ01;
	};
}

namespace tofu
{
	struct RenderingObject
	{
		uint32_t		materialType;
		ModelHandle		model;
		uint32_t		submesh;
		MaterialHandle	material;
		uint32_t		transformIdx;
		uint32_t		renderableId;
	};

	SINGLETON_IMPL(RenderingSystem);

	RenderingSystem::RenderingSystem()
		:
		modelHandleAlloc(),
		meshHandleAlloc(),
		materialHandleAlloc(),
		bufferHandleAlloc(),
		textureHandleAlloc(),
		samplerHandleAlloc(),
		vertexShaderHandleAlloc(),
		pixelShaderHandleAlloc(),
		pipelineStateHandleAlloc(),
		// levelResources(), // we don't need initialize this
		numLevelResources(),
		modelTable(),
		renderer(nullptr),
		frameNo(0),
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
		shadowSampler(),
		volumeSampler(),
		lutSampler(),
		builtinQuad(),
		builtinCube(),
		builtinSphere(),
		builtinCone(),
		defaultAlbedoMap(),
		defaultNormalMap(),
		defaultMetallicGlossMap(),
		defaultOcclusionMap(),
		lutMap(),
		cmdBuf(nullptr),
		// resources for deferred shading
		gBuffer1(),
		gBuffer2(),
		gBuffer3(), 
		gBuffer4()
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
		// Initialize Renderer Backend
		CHECKED(renderer->Init());

		//
		frameNo = 0;
		BeginFrame();

		// Create Built-in Shaders
		CHECKED(InitBuiltinMaterial(kMaterialSkybox,
			"assets/skybox_vs.shader",
			"assets/skybox_ps.shader"
		));

		//CHECKED(InitBuiltinMaterial(kMaterialTypeOpaque,
		//	"assets/opaque_vs.shader",
		//	"assets/opaque_ps.shader"
		//));

		//CHECKED(InitBuiltinMaterial(kMaterialTypeOpaqueSkinned,
		//	"assets/opaque_skinned_vs.shader",
		//	//"assets/opaque_ps.shader"
		//	"assets/opaque_skinned_ps.shader"
		//));


		//CHECKED(LoadVertexShader("assets/shadow_opaque_vs.shader", materialVSs[kMaterialTypeDepth]));
		//CHECKED(LoadVertexShader("assets/shadow_opaqueskinned_vs.shader", materialVSs[kMaterialTypeDepthSkinned]));


		CHECKED(LoadVertexShader("assets/shadow_vs.shader", materialVSs[kMaterialShadow]));
		CHECKED(LoadVertexShader("assets/shadow_skinned_vs.shader", materialVSs[kMaterialShadowSkinned]));

		CHECKED(InitBuiltinMaterial(kMaterialDeferredGeometryOpaque,
			"assets/deferred_geometry_vs.shader",
			"assets/deferred_geometry_ps.shader"));

		CHECKED(InitBuiltinMaterial(kMaterialDeferredGeometryOpaqueSkinned,
			"assets/deferred_geometry_skinned_vs.shader",
			"assets/deferred_geometry_ps.shader"));

		CHECKED(InitBuiltinMaterial(kMaterialDeferredLightingPointLight,
			"assets/deferred_lighting_vs.shader",
			"assets/deferred_lighting_pointlight_ps.shader"));

		CHECKED(LoadPixelShader("assets/deferred_lighting_spotlight_ps.shader", materialPSs[kMaterialDeferredLightingSpotLight]));

		CHECKED(InitBuiltinMaterial(kMaterialDeferredLightingAmbient,
			"assets/bypass_vs.shader",
			"assets/deferred_lighting_ambient_ps.shader"));

		CHECKED(LoadPixelShader("assets/deferred_transparent_ps.shader", materialPSs[kMaterialDeferredTransparent]));

		CHECKED(LoadPixelShader("assets/post_process_tone_mapping_ps.shader", materialPSs[kMaterialPostProcessToneMapping]));
		CHECKED(LoadPixelShader("assets/post_process_extract_bright_ps.shader", materialPSs[kMaterialPostProcessExtractBright]));
		CHECKED(LoadPixelShader("assets/post_process_blur_ps.shader", materialPSs[kMaterialPostProcessBlur]));

		CHECKED(LoadPixelShader("assets/volumetric_fog_apply_ps.shader", materialPSs[kMaterialPostProcessVolumetricFog]));

		CHECKED(InitBuiltinMaterial(kMaterialFontRendering,
			"assets/font_vs.shader",
			"assets/font_ps.shader"));

		CHECKED(LoadComputeShader("assets/volumetric_fog_inject_lights_cs.shader", injectionShader));
#if TOFU_FILTER_VFOG == 1
		CHECKED(LoadComputeShader("assets/volumetric_fog_filtering_cs.shader", filteringShader));
#endif
		CHECKED(LoadComputeShader("assets/volumetric_fog_scatter_cs.shader", scatterShader));

		// constant buffers
		{
			transformBufferSize = sizeof(math::float4x4) * 4 * kMaxEntities;
			transformBuffer = CreateConstantBuffer(transformBufferSize);

			frameConstantBuffer = CreateConstantBuffer(sizeof(FrameConstants));

			ambientDirLightBuffer = CreateConstantBuffer(sizeof(LightParametersDirectionalAndAmbient));

			pointLightBuffer = CreateConstantBuffer(sizeof(PointLightParams));

			spotLightBuffer = CreateConstantBuffer(sizeof(SpotLightParams));

			pointLightTransformBuffer = CreateConstantBuffer(sizeof(math::float4x4) * kMaxPointLights);

			spotLightTransformBuffer = CreateConstantBuffer(sizeof(math::float4x4) * kMaxSpotLights);

			shadowMatricesBuffer = CreateConstantBuffer(sizeof(math::float4x4) * kMaxShadowCastingLights * 4);

			fogParamsBuffer = CreateConstantBuffer(sizeof(VolumetricFogParams));
		}

		int32_t bufferWidth, bufferHeight;
		renderer->GetFrameBufferSize(bufferWidth, bufferHeight);

		// create built-in pipeline states
		{
			materialPSOs[kMaterialSkybox] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[kMaterialSkybox]);

			{
				CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
				params->handle = materialPSOs[kMaterialSkybox];
				params->vertexShader = materialVSs[kMaterialSkybox];
				params->pixelShader = materialPSs[kMaterialSkybox];

				params->cullMode = kCullFront;
				params->depthFunc = kComparisonLEqual;

				params->viewport = { 0.0f, 0.0f, float(bufferWidth), float(bufferHeight), 0.0f, 1.0f };

				params->label = kResourceGlobal;

				cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
			}

			materialPSOs[kMaterialShadow] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[kMaterialShadow]);
			{
				CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
				params->handle = materialPSOs[kMaterialShadow];
				params->vertexShader = materialVSs[kMaterialShadow];

				params->viewport = { 0.0f, 0.0f, 1024.0f, 1024.0f, 0.0f, 1.0f };

				params->label = kResourceGlobal;

				cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
			}

			materialPSOs[kMaterialShadowSkinned] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[kMaterialShadowSkinned]);
			{
				CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
				params->handle = materialPSOs[kMaterialShadowSkinned];
				params->vertexShader = materialVSs[kMaterialShadowSkinned];
				params->vertexFormat = kVertexFormatSkinned;

				params->viewport = { 0.0f, 0.0f, 1024.0f, 1024.0f, 0.0f, 1.0f };

				params->label = kResourceGlobal;

				cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
			}

			materialPSOs[kMaterialDeferredGeometryOpaque] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[kMaterialDeferredGeometryOpaque]);
			{
				CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
				params->handle = materialPSOs[kMaterialDeferredGeometryOpaque];
				params->vertexShader = materialVSs[kMaterialDeferredGeometryOpaque];
				params->pixelShader = materialPSs[kMaterialDeferredGeometryOpaque];
				//params->stencilEnable = 1;
				//params->frontFacePassOp = kStencilOpReplace;
				//params->stencilRef = 1u;

				params->viewport = { 0.0f, 0.0f, float(bufferWidth), float(bufferHeight), 0.0f, 1.0f };

				params->label = kResourceGlobal;

				cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
			}

			materialPSOs[kMaterialDeferredGeometryOpaqueSkinned] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[kMaterialDeferredGeometryOpaqueSkinned]);
			{
				CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
				params->handle = materialPSOs[kMaterialDeferredGeometryOpaqueSkinned];
				params->vertexShader = materialVSs[kMaterialDeferredGeometryOpaqueSkinned];
				params->pixelShader = materialPSs[kMaterialDeferredGeometryOpaqueSkinned];
				params->vertexFormat = kVertexFormatSkinned;
				//params->stencilEnable = 1;
				//params->frontFacePassOp = kStencilOpReplace;
				//params->stencilRef = 1u;

				params->viewport = { 0.0f, 0.0f, float(bufferWidth), float(bufferHeight), 0.0f, 1.0f };

				params->label = kResourceGlobal;

				cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
			}

			materialPSOs[kMaterialDeferredLightingOcclude] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[kMaterialDeferredLightingOcclude]);
			{
				CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
				params->handle = materialPSOs[kMaterialDeferredLightingOcclude];
				params->vertexShader = materialVSs[kMaterialDeferredLightingPointLight];
				params->pixelShader = PixelShaderHandle();// materialPSs[kMaterialDeferredLighting];

				params->depthEnable = 1;
				params->depthWrite = 0;

				params->stencilEnable = 1;

				params->backFaceDepthFailOp = kStencilOpInc;
				params->frontFaceDepthFailOp = kStencilOpDec;

				params->cullMode = kCullNone;

				params->viewport = { 0.0f, 0.0f, float(bufferWidth), float(bufferHeight), 0.0f, 1.0f };

				params->label = kResourceGlobal;

				cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
			}

			materialPSOs[kMaterialDeferredLightingPointLight] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[kMaterialDeferredLightingPointLight]);
			{
				CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
				params->handle = materialPSOs[kMaterialDeferredLightingPointLight];
				params->vertexShader = materialVSs[kMaterialDeferredLightingPointLight];
				params->pixelShader = materialPSs[kMaterialDeferredLightingPointLight];

				params->depthEnable = 0;

				params->stencilEnable = 1;
				params->frontFaceFunc = kComparisonNotEqual;
				params->backFaceFunc = kComparisonNotEqual;
				params->stencilRef = 0u;

				params->cullMode = kCullFront;

				params->blendEnable = 1;
				params->srcBlend = kBlendOne;
				params->destBlend = kBlendOne;

				params->viewport = { 0.0f, 0.0f, float(bufferWidth), float(bufferHeight), 0.0f, 1.0f };

				params->label = kResourceGlobal;

				cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
			}

			materialPSOs[kMaterialDeferredLightingSpotLight] = pipelineStateHandleAlloc.Allocate();
			assert(materialPSOs[kMaterialDeferredLightingSpotLight]);
			{
				CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
				params->handle = materialPSOs[kMaterialDeferredLightingSpotLight];
				params->vertexShader = materialVSs[kMaterialDeferredLightingPointLight];
				params->pixelShader = materialPSs[kMaterialDeferredLightingSpotLight];

				params->depthEnable = 0;

				params->stencilEnable = 1;
				params->frontFaceFunc = kComparisonNotEqual;
				params->backFaceFunc = kComparisonNotEqual;
				params->stencilRef = 0u;

				params->cullMode = kCullFront;

				params->blendEnable = 1;
				params->srcBlend = kBlendOne;
				params->destBlend = kBlendOne;

				params->viewport = { 0.0f, 0.0f, float(bufferWidth), float(bufferHeight), 0.0f, 1.0f };

				params->label = kResourceGlobal;

				cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
			}
		}

		materialPSOs[kMaterialDeferredLightingAmbient] = pipelineStateHandleAlloc.Allocate();
		assert(materialPSOs[kMaterialDeferredLightingAmbient]);
		{
			CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
			params->handle = materialPSOs[kMaterialDeferredLightingAmbient];
			params->vertexShader = materialVSs[kMaterialDeferredLightingAmbient];
			params->pixelShader = materialPSs[kMaterialDeferredLightingAmbient];

			params->depthEnable = 0;

			params->cullMode = kCullBack;

			params->blendEnable = 1;
			params->srcBlend = kBlendOne;
			params->destBlend = kBlendOne;

			params->viewport = { 0.0f, 0.0f, float(bufferWidth), float(bufferHeight), 0.0f, 1.0f };

			params->label = kResourceGlobal;

			cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
		}

		materialPSOs[kMaterialDeferredTransparent] = pipelineStateHandleAlloc.Allocate();
		assert(materialPSOs[kMaterialDeferredTransparent]);
		{
			CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
			params->handle = materialPSOs[kMaterialDeferredTransparent];
			params->vertexShader = materialVSs[kMaterialDeferredGeometryOpaque];
			params->pixelShader = materialPSs[kMaterialDeferredTransparent];

			params->depthWrite = 0;
			params->blendEnable = 1;
			params->srcBlend = kBlendSrcAlpha;
			params->destBlend = kBlendInvSrcAlpha;

			params->viewport = { 0.0f, 0.0f, float(bufferWidth), float(bufferHeight), 0.0f, 1.0f };

			params->label = kResourceGlobal;

			cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
		}

		materialPSOs[kMaterialDeferredTransparentSkinned] = pipelineStateHandleAlloc.Allocate();
		assert(materialPSOs[kMaterialDeferredTransparentSkinned]);
		{
			CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
			params->handle = materialPSOs[kMaterialDeferredTransparentSkinned];
			params->vertexShader = materialVSs[kMaterialDeferredGeometryOpaqueSkinned];
			params->pixelShader = materialPSs[kMaterialDeferredTransparent];
			params->vertexFormat = kVertexFormatSkinned;

			params->depthWrite = 0;
			params->blendEnable = 1;
			params->srcBlend = kBlendSrcAlpha;
			params->destBlend = kBlendInvSrcAlpha;

			params->viewport = { 0.0f, 0.0f, float(bufferWidth), float(bufferHeight), 0.0f, 1.0f };

			params->label = kResourceGlobal;

			cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
		}

		materialPSOs[kMaterialPostProcessToneMapping] = pipelineStateHandleAlloc.Allocate();
		assert(materialPSOs[kMaterialPostProcessToneMapping]);
		{
			CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
			params->handle = materialPSOs[kMaterialPostProcessToneMapping];
			params->vertexShader = materialVSs[kMaterialDeferredLightingAmbient];
			params->pixelShader = materialPSs[kMaterialPostProcessToneMapping];

			params->depthEnable = 0;

			params->cullMode = kCullBack;

			params->viewport = { 0.0f, 0.0f, float(bufferWidth), float(bufferHeight), 0.0f, 1.0f };

			params->label = kResourceGlobal;

			cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
		}

		materialPSOs[kMaterialPostProcessExtractBright] = pipelineStateHandleAlloc.Allocate();
		assert(materialPSOs[kMaterialPostProcessExtractBright]);
		{
			CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
			params->handle = materialPSOs[kMaterialPostProcessExtractBright];
			params->vertexShader = materialVSs[kMaterialDeferredLightingAmbient];
			params->pixelShader = materialPSs[kMaterialPostProcessExtractBright];

			params->depthEnable = 0;

			params->cullMode = kCullBack;

			params->viewport = { 0.0f, 0.0f, float(bufferWidth), float(bufferHeight), 0.0f, 1.0f };

			params->label = kResourceGlobal;

			cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
		}

		materialPSOs[kMaterialPostProcessBlur] = pipelineStateHandleAlloc.Allocate();
		assert(materialPSOs[kMaterialPostProcessBlur]);
		{
			CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
			params->handle = materialPSOs[kMaterialPostProcessBlur];
			params->vertexShader = materialVSs[kMaterialDeferredLightingAmbient];
			params->pixelShader = materialPSs[kMaterialPostProcessBlur];

			params->depthEnable = 0;
			params->blendEnable = 1;
			params->srcBlend = kBlendOne;
			params->destBlend = kBlendOne;
			params->srcBlendAlpha = kBlendOne;
			params->destBlendAlpha = kBlendOne;

			params->cullMode = kCullBack;

			params->viewport = { 0.0f, 0.0f, float(bufferWidth), float(bufferHeight), 0.0f, 1.0f };

			params->label = kResourceGlobal;

			cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
		}

		materialPSOs[kMaterialPostProcessVolumetricFog] = pipelineStateHandleAlloc.Allocate();
		assert(materialPSOs[kMaterialPostProcessVolumetricFog]);
		{
			CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
			params->handle = materialPSOs[kMaterialPostProcessVolumetricFog];
			params->vertexShader = materialVSs[kMaterialDeferredLightingAmbient];
			params->pixelShader = materialPSs[kMaterialPostProcessVolumetricFog];

			params->depthEnable = 0;

			params->cullMode = kCullBack;

			params->viewport = { 0.0f, 0.0f, float(bufferWidth), float(bufferHeight), 0.0f, 1.0f };

			params->label = kResourceGlobal;

			cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
		}

		materialPSOs[kMaterialFontRendering] = pipelineStateHandleAlloc.Allocate();
		assert(materialPSOs[kMaterialFontRendering]);
		{
			CreatePipelineStateParams* params = MemoryAllocator::FrameAlloc<CreatePipelineStateParams>();
			params->handle = materialPSOs[kMaterialFontRendering];
			params->vertexShader = materialVSs[kMaterialFontRendering];
			params->pixelShader = materialPSs[kMaterialFontRendering];

			params->depthEnable = 0;
			params->blendEnable = 1;
			params->srcBlend = kBlendSrcAlpha;
			params->destBlend = kBlendInvSrcAlpha;

			params->cullMode = kCullBack;

			params->frontFaceCcw = 1;

			params->viewport = { 0.0f, 0.0f, float(bufferWidth), float(bufferHeight), 0.0f, 1.0f };
			params->vertexFormat = kVertexFormatOverlay;

			params->label = kResourceGlobal;

			cmdBuf->Add(RendererCommand::kCommandCreatePipelineState, params);
		}

		// create default sampler
		{
			defaultSampler = samplerHandleAlloc.Allocate();
			assert(defaultSampler);

			{
				CreateSamplerParams* params = MemoryAllocator::FrameAlloc<CreateSamplerParams>();
				params->handle = defaultSampler;
				params->filter = kTextureFilterAnisotropic;
				params->maxAnisotropy = 7;
				params->textureAddressU = kTextureAddressWarp;
				params->textureAddressV = kTextureAddressWarp;
				params->textureAddressW = kTextureAddressWarp;
				params->label = kResourceGlobal;
				cmdBuf->Add(RendererCommand::kCommandCreateSampler, params);
			}
		}
		// Create shadow sampler
		{
			shadowSampler = samplerHandleAlloc.Allocate();
			assert(shadowSampler);

			{
				CreateSamplerParams* params = MemoryAllocator::FrameAlloc<CreateSamplerParams>();
				params->handle = shadowSampler;
				params->textureAddressU = kTextureAddressClamp;
				params->textureAddressV = kTextureAddressClamp;
				params->textureAddressW = kTextureAddressClamp;
				params->label = kResourceGlobal;

				cmdBuf->Add(RendererCommand::kCommandCreateSampler, params);
			}
		}
		// Create volume sampler
		{
			volumeSampler = samplerHandleAlloc.Allocate();
			assert(volumeSampler);

			{
				CreateSamplerParams* params = MemoryAllocator::FrameAlloc<CreateSamplerParams>();
				params->handle = volumeSampler;
				params->textureAddressU = kTextureAddressClamp;
				params->textureAddressV = kTextureAddressClamp;
				params->textureAddressW = kTextureAddressClamp;
				params->label = kResourceGlobal;

				cmdBuf->Add(RendererCommand::kCommandCreateSampler, params);
			}
		}
		// Create lut sampler
		{
			lutSampler = samplerHandleAlloc.Allocate();
			assert(lutSampler);

			{
				CreateSamplerParams* params = MemoryAllocator::FrameAlloc<CreateSamplerParams>();
				params->handle = lutSampler;
				params->textureAddressU = kTextureAddressClamp;
				params->textureAddressV = kTextureAddressClamp;
				params->textureAddressW = kTextureAddressClamp;
				params->label = kResourceGlobal;
				cmdBuf->Add(RendererCommand::kCommandCreateSampler, params);
			}
		}


		builtinQuad = CreateModel("assets/quad.model", kResourceGlobal);
		assert(nullptr != builtinQuad);

		builtinCube = CreateModel("assets/cube.model", kResourceGlobal);
		assert(nullptr != builtinCube);

		builtinSphere = CreateModel("assets/sphere.model", kResourceGlobal);
		assert(nullptr != builtinSphere);

		builtinCone = CreateModel("assets/cone.model", kResourceGlobal);
		assert(nullptr != builtinCone);

		{
			math::float4* data1 = reinterpret_cast<math::float4*>(
				MemoryAllocator::Allocators[kAllocGlobal].Allocate(
					sizeof(math::float4), sizeof(math::float4)));

			math::float4* data2 = reinterpret_cast<math::float4*>(
				MemoryAllocator::Allocators[kAllocGlobal].Allocate(
					sizeof(math::float4), sizeof(math::float4)));

			math::float4* data3 = reinterpret_cast<math::float4*>(
				MemoryAllocator::Allocators[kAllocGlobal].Allocate(
					sizeof(math::float4), sizeof(math::float4)));

			if (nullptr == data1 || nullptr == data2 || nullptr == data3)
			{
				return kErrUnknown;
			}

			*(data1) = math::float4{ 1, 1, 1, 1 };

			*(data2) = math::float4{ 0.5f, 0.5f, 1, 1 };

			*(data3) = math::float4{ 0, 0, 0, 0 };

			TextureHandle whiteTex = RenderingSystem::instance()->CreateTexture(kFormatR32g32b32a32Float, 1, 1, 1, 16, data1, kBindingShaderResource, kResourceGlobal);
			if (!whiteTex)
				return kErrUnknown;

			TextureHandle normalUpTex = RenderingSystem::instance()->CreateTexture(kFormatR32g32b32a32Float, 1, 1, 1, 16, data2, kBindingShaderResource, kResourceGlobal);
			if (!normalUpTex)
				return kErrUnknown;

			TextureHandle blackTex = RenderingSystem::instance()->CreateTexture(kFormatR32g32b32a32Float, 1, 1, 1, 16, data3, kBindingShaderResource, kResourceGlobal);
			if (!normalUpTex)
				return kErrUnknown;

			defaultAlbedoMap = whiteTex;
			defaultNormalMap = normalUpTex;
			defaultMetallicGlossMap = blackTex;
			defaultOcclusionMap = whiteTex;
			defaultEmissionMap = blackTex;

			lutMap = CreateTexture("assets/textures/test/BrdfLUT-Copy.texture", kResourceGlobal);
		}

		// TODO when back buffer size changed !
		int32_t w, h;
		renderer->GetFrameBufferSize(w, h);
		gBuffer1 = CreateTexture(kFormatR32g32b32a32Float, w, h, 1, 0, nullptr, kBindingRenderTarget | kBindingShaderResource, kResourceGlobal);
		gBuffer2 = CreateTexture(kFormatR32g32b32a32Float, w, h, 1, 0, nullptr, kBindingRenderTarget | kBindingShaderResource, kResourceGlobal);
		gBuffer3 = CreateTexture(kFormatR32g32b32a32Float, w, h, 1, 0, nullptr, kBindingRenderTarget | kBindingShaderResource, kResourceGlobal);
		gBuffer4 = CreateTexture(kFormatR32g32b32a32Float, w, h, 1, 0, nullptr, kBindingRenderTarget | kBindingShaderResource, kResourceGlobal);

		hdrTarget = CreateTexture(kFormatR32g32b32a32Float, w, h, 1, 0, nullptr, kBindingRenderTarget | kBindingShaderResource, kResourceGlobal);
		hdrTarget2 = CreateTexture(kFormatR32g32b32a32Float, w, h, 1, 0, nullptr, kBindingRenderTarget | kBindingShaderResource, kResourceGlobal);

		if (!gBuffer1 || !gBuffer2 || !gBuffer3 || !gBuffer4 || !hdrTarget || !hdrTarget2)
		{
			return kErrUnknown;
		}

		shadowMapArray = CreateTexture(kFormatD24UnormS8Uint, 1024, 1024, kMaxShadowCastingLights, 0, nullptr, kBindingShaderResource | kBindingDepthStencil, kResourceGlobal);

		if (!shadowMapArray) return kErrUnknown;

		for (uint32_t i = 0; i < kMaxShadowCastingLights; i++)
		{
			shadowMaps[i] = CreateTexture(shadowMapArray, i, 1, kBindingDepthStencil, kFormatAuto, kResourceGlobal);
			if (!shadowMaps[i]) return kErrUnknown;
		}

		injectionTex = CreateTexture(kFormatR32g32b32a32Float, 160, 90, 128, 0, 0, nullptr, kBindingShaderResource | kBindingUnorderedAccess, kResourceGlobal);
#if TOFU_FILTER_VFOG == 1
		filteredVolumeTex = CreateTexture(kFormatR32g32b32a32Float, 160, 90, 128, 0, 0, nullptr, kBindingShaderResource | kBindingUnorderedAccess, kResourceGlobal);
		if (!filteredVolumeTex)
			return kErrUnknown;
#endif
		scatterTex = CreateTexture(kFormatR32g32b32a32Float, 160, 90, 128, 0, 0, nullptr, kBindingShaderResource | kBindingUnorderedAccess, kResourceGlobal);

		if (!injectionTex || !scatterTex)
		{
			return kErrUnknown;
		}

		{
			uint32_t maxVerts = 2048;
			uint32_t vertSize = sizeof(float) * 9u;
			TextureHandle fontAtlas = textureHandleAlloc.Allocate();
			BufferHandle vb = bufferHandleAlloc.Allocate();
			BufferHandle ib = bufferHandleAlloc.Allocate();
			if (!fontAtlas || !vb || !ib)
				return kErrUnknown;

			{
				CreateBufferParams* params = MemoryAllocator::FrameAlloc<CreateBufferParams>();
				params->handle = vb;
				params->bindingFlags = kBindingVertexBuffer;
				params->dynamic = 1;
				params->data = nullptr;
				params->size = vertSize * maxVerts;
				params->stride = vertSize;
				params->label = kResourceGlobal;

				cmdBuf->Add(RendererCommand::kCommandCreateBuffer, params);
			}

			{
				CreateBufferParams* params = MemoryAllocator::FrameAlloc<CreateBufferParams>();
				params->handle = ib;
				params->bindingFlags = kBindingIndexBuffer;
				params->dynamic = 1;
				params->data = nullptr;
				params->size = sizeof(uint16_t) * maxVerts;
				params->label = kResourceGlobal;

				cmdBuf->Add(RendererCommand::kCommandCreateBuffer, params);
			}

			fontRenderer.Setup(materialPSOs[kMaterialFontRendering], 
				fontAtlas, 
				volumeSampler, 
				vb, ib, frameConstantBuffer, maxVerts);

			CHECKED(fontRenderer.Reset(cmdBuf));
			CHECKED(fontRenderer.Init());
		}

		return kOK;
	}

	int32_t RenderingSystem::Shutdown()
	{
		CHECKED(fontRenderer.Shutdown());

		CHECKED(renderer->Release());

		return kOK;
	}

	int32_t RenderingSystem::BeginFrame()
	{
		if (nullptr != cmdBuf)
		{
			return kOK;
		}

		//assert(false && "sync need to be done.");

		cmdBuf = RendererCommandBuffer::Create(kCommandBufferCapacity);
		assert(nullptr != cmdBuf);

		CHECKED(fontRenderer.Reset(cmdBuf));

		return kOK;
	}

	int32_t RenderingSystem::Update()
	{
		CHECKED(DebugPipeline());
		//CHECKED(DeferredPipeline());

		CHECKED(fontRenderer.Submit());
		return kOK;
	}

	int32_t RenderingSystem::EndFrame()
	{
		// submit command buffer
		CHECKED(renderer->Submit(cmdBuf));

		return kOK;
	}

	int32_t RenderingSystem::SwapBuffers()
	{
		// back buffer swap
		CHECKED(renderer->Present());

		frameNo++;

		cmdBuf = nullptr;

		return kOK;
	}

	Model* RenderingSystem::CreateModel(const char* filename, uint32_t label)
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

		uint32_t allocSlot = kAllocLevel;
		if (label == kResourceGlobal) allocSlot = kAllocGlobal;

		int32_t err = FileIO::ReadFile(filename, false, 4, allocSlot, reinterpret_cast<void**>(&data), &size);
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

		//uint32_t verticesCount = 0;
		uint32_t indicesCount = 0;

		assert(header->NumMeshes <= kMaxMeshesPerModel);
		ModelHandle modelHandle = modelHandleAlloc.Allocate();
		assert(modelHandle);
		Model& model = models[modelHandle.id];

		model = Model();
		model.handle = modelHandle;
		model.numVertices = header->NumVertices;
		model.numMeshes = header->NumMeshes;
		model.vertexSize = header->CalculateVertexSize();
		model.rawData = data;
		model.rawDataSize = size;
		model.header = header;

		if (label == kResourceLevel)
			AddLevelResourceHandle(modelHandle);

		//verticesCount = header->NumVertices;

		// allocate vertex buffer and index buffer
		BufferHandle vbHandle = bufferHandleAlloc.Allocate();
		BufferHandle ibHandle = bufferHandleAlloc.Allocate();
		assert(vbHandle && ibHandle);

		if (label == kResourceLevel)
		{
			AddLevelResourceHandle(vbHandle);
			AddLevelResourceHandle(ibHandle);
		}

		// store mesh infos
		for (uint32_t i = 0; i < header->NumMeshes; ++i)
		{
			model.meshes[i] = meshHandleAlloc.Allocate();
			assert(model.meshes[i]);
			uint32_t id = model.meshes[i].id;
			meshes[id] = Mesh();
			meshes[id].VertexBuffer = vbHandle;
			meshes[id].IndexBuffer = ibHandle;
			meshes[id].StartVertex = meshInfos[i].BaseVertex;
			meshes[id].StartIndex = indicesCount;
			meshes[id].NumIndices = meshInfos[i].NumIndices;

			indicesCount += meshInfos[i].NumIndices;

			if (label == kResourceLevel)
				AddLevelResourceHandle(model.meshes[i]);
		}

		// aligned to dword
		if (indicesCount % 2 != 0)
		{
			indicesCount += 1;
		}

		// 
		uint32_t vertexBufferSize = model.numVertices * model.vertexSize;
		uint32_t indexBufferSize = indicesCount * sizeof(uint16_t);

		uint8_t* vertices = reinterpret_cast<uint8_t*>(meshInfos + header->NumMeshes);
		uint8_t* indices = vertices + vertexBufferSize;

		model.vertices = reinterpret_cast<float*>(vertices);

		for (uint32_t i = 0; i < header->NumMeshes; ++i)
		{
			uint32_t id = model.meshes[i].id;
			model.baseVertex[i] = meshes[id].StartVertex;
			model.numIndices[i] = meshes[id].NumIndices;
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
				model.bones[iBone].transformMatrix = math::transpose(model.bones[iBone].transformMatrix);
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


				model.frames = reinterpret_cast<model::ModelAnimFrame*>(
					model.animations + header->NumAnimations
					);
			}
		}

		// upload vertices and indices to vertex buffer and index buffer
		{
			CreateBufferParams* params = MemoryAllocator::FrameAlloc<CreateBufferParams>();
			params->handle = vbHandle;
			params->bindingFlags = kBindingVertexBuffer;
			params->data = vertices;
			params->size = vertexBufferSize;
			params->stride = header->CalculateVertexSize();
			params->label = label;

			cmdBuf->Add(RendererCommand::kCommandCreateBuffer, params);
		}

		{
			CreateBufferParams* params = MemoryAllocator::FrameAlloc<CreateBufferParams>();
			params->handle = ibHandle;
			params->bindingFlags = kBindingIndexBuffer;
			params->data = indices;
			params->size = indexBufferSize;
			params->label = label;

			cmdBuf->Add(RendererCommand::kCommandCreateBuffer, params);
		}

		modelTable.insert(std::pair<std::string, ModelHandle>(strFilename, modelHandle));

		return &model;
	}

	TextureHandle RenderingSystem::CreateTexture(const char* filename, uint32_t label)
	{
		void* data = nullptr;
		size_t size = 0;
		int32_t err = FileIO::ReadFile(filename, false, 4, &data, &size);

		if (kOK != err)
		{
			return TextureHandle();
		}

		TextureHandle handle = textureHandleAlloc.Allocate();
		if (!handle)
		{
			return TextureHandle();
		}

		CreateTextureParams* params = MemoryAllocator::FrameAlloc<CreateTextureParams>();

		params->handle = handle;
		params->bindingFlags = kBindingShaderResource;
		params->isFile = 1;
		params->data = data;
		params->width = static_cast<uint32_t>(size);
		params->label = label;

		cmdBuf->Add(RendererCommand::kCommandCreateTexture, params);

		if (label == kResourceLevel)
			AddLevelResourceHandle(handle);

		return handle;
	}

	TextureHandle RenderingSystem::CreateTexture(PixelFormat format, uint32_t width, uint32_t height, uint32_t arraySize, uint32_t pitch, void* data, uint32_t binding, uint32_t label)
	{
		TextureHandle handle = textureHandleAlloc.Allocate();
		if (!handle)
		{
			return handle;
		}

		CreateTextureParams* params = MemoryAllocator::FrameAlloc<CreateTextureParams>();

		params->InitAsTexture2D(handle, width, height, format, data, pitch, arraySize, binding);
		params->label = label;

		cmdBuf->Add(RendererCommand::kCommandCreateTexture, params);

		if (label == kResourceLevel)
			AddLevelResourceHandle(handle);

		return handle;
	}

	TextureHandle RenderingSystem::CreateTexture(TextureHandle source, uint32_t startIndex, uint32_t arraySize, uint32_t binding, PixelFormat format, uint32_t label)
	{
		TextureHandle handle = textureHandleAlloc.Allocate();
		if (!handle)
		{
			return handle;
		}

		CreateTextureParams* params = MemoryAllocator::FrameAlloc<CreateTextureParams>();

		params->InitAsTexture2DSlice(handle, source, startIndex, arraySize, format, binding);
		params->label = label;

		cmdBuf->Add(RendererCommand::kCommandCreateTexture, params);

		if (label == kResourceLevel)
			AddLevelResourceHandle(handle);

		return handle;
	}

	TextureHandle RenderingSystem::CreateTexture(PixelFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t pitch, uint32_t slicePitch, void* data, uint32_t binding, uint32_t label)
	{
		TextureHandle handle = textureHandleAlloc.Allocate();
		if (!handle)
		{
			return handle;
		}

		CreateTextureParams* params = MemoryAllocator::FrameAlloc<CreateTextureParams>();

		params->InitAsTexture3D(handle, width, height, depth, format, data, pitch, slicePitch, binding);
		params->label = label;

		cmdBuf->Add(RendererCommand::kCommandCreateTexture, params);

		if (label == kResourceLevel)
			AddLevelResourceHandle(handle);

		return handle;
	}

	Material* RenderingSystem::CreateMaterial(MaterialType type, uint32_t label)
	{
		MaterialHandle handle = materialHandleAlloc.Allocate();
		if (!handle)
		{
			return nullptr;
		}

		Material* mat = &(materials[handle.id]);
		new (mat) Material(type);
		mat->handle = handle;

		if (label == kResourceLevel)
			AddLevelResourceHandle(handle);

		mat->materialParamsBuffer = CreateConstantBuffer(sizeof(MaterialParams), true, label);

		return mat;
	}

	int32_t RenderingSystem::CleanupLevelResources()
	{
		std::sort(levelResources, levelResources + numLevelResources, [](BaseHandle a, BaseHandle b)
		{
			return a.raw < b.raw;
		});

		uint32_t i = 0;

#define ReleaseHandle(TYPE, HANDLE_ALLOC, HANDLE_TYPE) \
		for (; levelResources[i].type == TYPE; i++) \
		{ \
			HANDLE_ALLOC.Free(HANDLE_TYPE(levelResources[i].id)); \
		}

		ReleaseHandle(kHandleTypeBuffer, bufferHandleAlloc, BufferHandle);
		ReleaseHandle(kHandleTypeTexture, textureHandleAlloc, TextureHandle);
		ReleaseHandle(kHandleTypeSampler, samplerHandleAlloc, SamplerHandle);
		ReleaseHandle(kHandleTypeVertexShader, vertexShaderHandleAlloc, VertexShaderHandle);
		ReleaseHandle(kHandleTypePixelShader, pixelShaderHandleAlloc, PixelShaderHandle);
		ReleaseHandle(kHandleTypeComputeShader, computeShaderHandleAlloc, ComputeShaderHandle);
		ReleaseHandle(kHandleTypePipelineState, pipelineStateHandleAlloc, PipelineStateHandle);


		ReleaseHandle(kHandleTypeMesh, meshHandleAlloc, MeshHandle);
		ReleaseHandle(kHandleTypeModel, modelHandleAlloc, ModelHandle);
		ReleaseHandle(kHandleTypeMaterial, materialHandleAlloc, MaterialHandle);

#undef ReleaseHandle

		modelTable.clear();
		numLevelResources = 0;

		return renderer->CleanupResources(kResourceLevel);
	}

	int32_t RenderingSystem::RenderText(const char * text, float x, float y)
	{
		return fontRenderer.Render(text, x, y);
	}

	float RenderingSystem::GetGPUTime()
	{
		return renderer->GetGPUTime(0);
	}

	BufferHandle RenderingSystem::CreateConstantBuffer(uint32_t size, bool dynamic, uint32_t label)
	{
		BufferHandle handle = bufferHandleAlloc.Allocate();
		assert(handle);

		{
			CreateBufferParams* params = MemoryAllocator::FrameAlloc<CreateBufferParams>();
			assert(nullptr != params);
			params->handle = handle;
			params->bindingFlags = kBindingConstantBuffer;
			params->size = size;
			params->dynamic = dynamic ? 1 : 0;
			params->label = label;

			cmdBuf->Add(RendererCommand::kCommandCreateBuffer, params);
		}

		if (label == kResourceLevel)
			AddLevelResourceHandle(handle);

		return handle;
	}

	int32_t RenderingSystem::UpdateConstantBuffer(BufferHandle buffer, uint32_t size, void* data)
	{
		UpdateBufferParams* params = MemoryAllocator::FrameAlloc<UpdateBufferParams>();
		assert(nullptr != params);

		params->handle = buffer;
		params->data = data;
		params->size = size;

		cmdBuf->Add(RendererCommand::kCommandUpdateBuffer, params);

		return kOK;
	}

	int32_t RenderingSystem::InitBuiltinMaterial(MaterialType matType, const char * vsFile, const char * psFile)
	{
		if (materialVSs[matType] || materialPSs[matType])
			return kErrUnknown;

		CHECKED(LoadVertexShader(vsFile, materialVSs[matType]));
		CHECKED(LoadPixelShader(psFile, materialPSs[matType]));

		return kOK;
	}

	int32_t RenderingSystem::LoadVertexShader(const char * filename, VertexShaderHandle & handle, uint32_t label)
	{
		handle = vertexShaderHandleAlloc.Allocate();
		if (!handle)
		{
			return kErrUnknown;
		}

		{
			CreateVertexShaderParams* params = MemoryAllocator::FrameAlloc<CreateVertexShaderParams>();
			assert(nullptr != params);
			params->handle = handle;

			int32_t err = FileIO::ReadFile(
				filename,
				false,
				4,
				&(params->data),
				&(params->size));

			if (kOK != err)
			{
				return err;
			}

			params->label = label;

			cmdBuf->Add(RendererCommand::kCommandCreateVertexShader, params);
		}

		if (label == kResourceLevel)
			AddLevelResourceHandle(handle);

		return kOK;
	}

	int32_t RenderingSystem::LoadPixelShader(const char * filename, PixelShaderHandle & handle, uint32_t label)
	{
		handle = pixelShaderHandleAlloc.Allocate();
		if (!handle)
		{
			return kErrUnknown;
		}

		{
			CreatePixelShaderParams* params = MemoryAllocator::FrameAlloc<CreatePixelShaderParams>();
			assert(nullptr != params);
			params->handle = handle;

			int32_t err = FileIO::ReadFile(
				filename,
				false,
				4,
				&(params->data),
				&(params->size));

			if (kOK != err)
			{
				return err;
			}

			params->label = label;

			cmdBuf->Add(RendererCommand::kCommandCreatePixelShader, params);
		}

		if (label == kResourceLevel)
			AddLevelResourceHandle(handle);

		return kOK;
	}

	int32_t RenderingSystem::LoadComputeShader(const char * filename, ComputeShaderHandle & handle, uint32_t label)
	{
		handle = computeShaderHandleAlloc.Allocate();
		if (!handle)
		{
			return kErrUnknown;
		}

		{
			CreateComputeShaderParams* params = MemoryAllocator::FrameAlloc<CreateComputeShaderParams>();
			assert(nullptr != params);
			params->handle = handle;

			int32_t err = FileIO::ReadFile(
				filename,
				false,
				4,
				&(params->data),
				&(params->size));

			if (kOK != err)
			{
				return err;
			}

			params->label = label;

			cmdBuf->Add(RendererCommand::kCommandCreateComputeShader, params);
		}

		if (label == kResourceLevel)
			AddLevelResourceHandle(handle);

		return kOK;
	}

	int32_t RenderingSystem::ReallocAnimationResources(AnimationComponentData & c)
	{
		Model* model = c.model;
		if (nullptr == model || nullptr == model->header || !model->HasAnimation() || 0 == model->header->NumBones)
		{
			return kErrUnknown;
		}

		if (!c.boneMatricesBuffer)
		{
			BufferHandle handle = CreateConstantBuffer(sizeof(math::float4x4) * 256, true, kResourceLevel);
			assert(handle);
			c.boneMatricesBuffer = handle;
		}

		c.boneMatricesBufferSize = static_cast<uint32_t>(sizeof(math::float4x4)) * model->header->NumBones;

		// TODO:
		c.ReallocResources();

		return kOK;
	}

	int32_t RenderingSystem::DeferredPipeline()
	{
		if (CameraComponent::GetNumActiveComponents() == 0)
		{
			return kOK;
		}
		CameraComponentData& camera = CameraComponent::GetAllComponents()[0];

		// update aspect
		int32_t bufferWidth, bufferHeight;
		{
			renderer->GetFrameBufferSize(bufferWidth, bufferHeight);
			camera.SetAspect(bufferHeight == 0 ? 1.0f : float(bufferWidth) / bufferHeight);
		}

		Frustum camFrustum;
		math::float3 camPos;
		float farClipZ = 0.0f;
		float nearClipZ = 0.0f;
		{
			FrameConstants* data = reinterpret_cast<FrameConstants*>(
				MemoryAllocator::FrameAlloc(sizeof(FrameConstants), 4)
				);
			assert(nullptr != data);

#ifdef TOFU_USE_GLM
			math::float4x4 matView = camera.CalcViewMatrix();
			math::float4x4 matProj = camera.CalcProjectionMatrix();
			data->matView = math::transpose(matView);
			data->matProj = math::transpose(matProj);
			data->matViewInv = math::transpose(math::inverse(matView));
			data->matProjInv = math::transpose(math::inverse(matProj));
#else
			data->matView = camera.CalcViewMatrix();
			data->matProj = camera.CalcProjectionMatrix();
			data->matViewInv = math::inverse(data->matView);
			data->matProjInv = math::inverse(data->matProj);
#endif

			TransformComponent t = camera.entity.GetComponent<TransformComponent>();
			data->cameraPos = math::float4(t->GetWorldPosition(), 1);
			data->bufferSize = math::float4(float(bufferWidth), float(bufferHeight), 0, 0);

			data->perspectiveParams = math::float4{ camera.GetFOV(), camera.GetAspect(), camera.GetZNear(), camera.GetZFar() };
			farClipZ = data->perspectiveParams.w;
			nearClipZ = data->perspectiveParams.z;
			float tanHalfFOV = math::tan(math::radians(data->perspectiveParams.x) * 0.5f);
			float farClipMaxY = tanHalfFOV * farClipZ;
			float farClipMaxX = farClipMaxY * data->perspectiveParams.y;

			const Transform& worldTrans = t->GetWorldTransform();
			data->leftTopRay = worldTrans.TransformVector(math::float4{ -farClipMaxX, farClipMaxY, farClipZ, 0 });
			data->rightTopRay = worldTrans.TransformVector(math::float4{ farClipMaxX, farClipMaxY, farClipZ, 0 });
			data->leftBottomRay = worldTrans.TransformVector(math::float4{ -farClipMaxX, -farClipMaxY, farClipZ, 0 });
			data->rightBottomRay = worldTrans.TransformVector(math::float4{ farClipMaxX, -farClipMaxY, farClipZ, 0 });

			camFrustum.near = nearClipZ;
			camFrustum.far = farClipZ;
			camFrustum.topSlope = tanHalfFOV;
			camFrustum.bottomSlope = -tanHalfFOV;
			camFrustum.rightSlope = tanHalfFOV * data->perspectiveParams.y;
			camFrustum.leftSlope = -camFrustum.rightSlope;
			camFrustum.Transform(t->GetWorldTransform());

			camPos = data->cameraPos;

			UpdateConstantBuffer(frameConstantBuffer, sizeof(FrameConstants), data);
		}

		// clear default render target and depth buffer
		{
			ClearParams* params = MemoryAllocator::FrameAlloc<ClearParams>();

			params->renderTargets[0] = hdrTarget;
			params->clearColor[0] = 0;
			params->clearColor[1] = 0;
			params->clearColor[2] = 0;
			params->clearColor[3] = 1;

			//params->clearStencil = 1u;

			cmdBuf->Add(RendererCommand::kCommandClearRenderTargets, params);
		}

		for (uint32_t i = 0; i < kMaxMaterials; i++)
		{
			MaterialHandle handle{ i };
			if (materialHandleAlloc.IsValid(handle) && materials[i].isDirty)
			{
				assert(materials[i].materialParamsBuffer);

				void* buffer = MemoryAllocator::FrameAlloc(sizeof(MaterialParams), 4);
				memcpy(buffer, &(materials[i].materialParams), sizeof(MaterialParams));

				UpdateConstantBuffer(materials[i].materialParamsBuffer,
					sizeof(MaterialParams),
					buffer);
				materials[i].isDirty = false;
			}
		}

		// get all renderables in system
		RenderingComponentData* renderables = RenderingComponent::GetAllComponents();
		uint32_t renderableCount = RenderingComponent::GetNumActiveComponents();

		// buffer for transform matrices
		math::float4x4* transformArray = reinterpret_cast<math::float4x4*>(
			MemoryAllocator::FrameAlloc(transformBufferSize, 4)
			);


		// list of active renderables (used for culling)
		RenderingObject* activeObjects = reinterpret_cast<RenderingObject*>(
			MemoryAllocator::FrameAlloc(sizeof(RenderingObject) * kMaxEntities * 2, 4)
			);

		/*uint32_t* opaqueObjects = reinterpret_cast<uint32_t*>(
			MemoryAllocator::FrameAlloc(sizeof(uint32_t) * kMaxEntities, 4)
			);

		uint32_t* transparentObjects = reinterpret_cast<uint32_t*>(
			MemoryAllocator::FrameAlloc(sizeof(uint32_t) * kMaxEntities, 4)
			);*/

		assert(nullptr != transformArray && nullptr != activeObjects);

		uint32_t numActiveEntities = 0;
		uint32_t numActiveObjects = 0;
		uint32_t numOpaqueObjects = 0;
		uint32_t numTransparentObjects = 0;

		// fill in transform matrix for active renderables
		for (uint32_t i = 0; i < renderableCount; ++i)
			// for (uint32_t i = renderableCount - 2; i < renderableCount; ++i)
		{
			RenderingComponentData& comp = renderables[i];
			
			TransformComponent transform = comp.entity.GetComponent<TransformComponent>();
			assert(transform);

			// TODO culling

			// fill transform matrices;
			uint32_t idx = numActiveEntities++;

#ifdef TOFU_USE_GLM
			transformArray[idx * 4] = math::transpose(transform->GetWorldTransform().GetMatrix());
#else
			transformArrayOpaque[idx * 4] = transform->GetWorldTransform().GetMatrix();
#endif
			transformArray[idx * 4 + 1] = math::transpose(math::inverse(transformArray[idx * 4]));

			// add all submeshes into the queue
			uint32_t numMeshes = comp.model->numMeshes;
			uint32_t numMaterials = comp.numMaterials;
			for (uint32_t j = 0; j < numMeshes; ++j)
			{
				Material* mat = comp.materials[j < numMaterials ? j : (numMaterials - 1)];

				RenderingObject& obj = activeObjects[numActiveObjects++];

				obj.model = comp.model->handle;
				obj.material = mat->handle;
				obj.materialType = mat->type;
				obj.submesh = j;
				obj.renderableId = i;
				obj.transformIdx = idx;

				if (obj.materialType == kMaterialDeferredTransparent || obj.materialType == kMaterialDeferredTransparentSkinned)
				{
					numTransparentObjects++;
				}
				else
				{
					numOpaqueObjects++;
				}
			}
		}

		// sorting rendering objects
		std::sort(activeObjects, activeObjects + numActiveObjects, [](RenderingObject& a, RenderingObject& b)
		{
			return a.materialType < b.materialType;
		});

		// upload transform matrices
		if (numActiveEntities > 0)
		{
			UpdateConstantBuffer(transformBuffer, sizeof(math::float4x4) * 4 * numActiveEntities, transformArray);
		}

		// get all lights
		LightComponentData* lights = LightComponent::GetAllComponents();
		uint32_t lightsCount = LightComponent::GetNumActiveComponents();

		assert(lightsCount <= kMaxEntities);

		LightParametersDirectionalAndAmbient* ambDirLightParams = reinterpret_cast<LightParametersDirectionalAndAmbient*>(
			MemoryAllocator::FrameAlloc(sizeof(LightParametersDirectionalAndAmbient), 16)
			);
		assert(nullptr != ambDirLightParams);

		PointLightParams* pointLightParams = reinterpret_cast<PointLightParams*>(
			MemoryAllocator::FrameAlloc(sizeof(PointLightParams), 16)
			);
		assert(nullptr != pointLightParams);

		SpotLightParams* spotLightParams = reinterpret_cast<SpotLightParams*>(
			MemoryAllocator::FrameAlloc(sizeof(SpotLightParams), 16)
			);
		assert(nullptr != spotLightParams);

		math::float4x4* pointLightTransform = reinterpret_cast<math::float4x4*>(
			MemoryAllocator::FrameAlloc(sizeof(math::float4x4) * kMaxPointLights, 16)
			);
		assert(nullptr != pointLightTransform);

		math::float4x4* spotLightTransform = reinterpret_cast<math::float4x4*>(
			MemoryAllocator::FrameAlloc(sizeof(math::float4x4) * kMaxSpotLights, 16)
			);
		assert(nullptr != spotLightTransform);

		math::float4x4* shadowMatrices = reinterpret_cast<math::float4x4*>(
			MemoryAllocator::FrameAlloc(sizeof(math::float4x4) * kMaxShadowCastingLights * 4, 16)
			);
		assert(nullptr != shadowMatrices);


		//uint32_t* pointLights = reinterpret_cast<uint32_t*>(MemoryAllocator::FrameAlloc(sizeof(uint32_t) * kMaxEntities, 4));
		//uint32_t* spotLights = reinterpret_cast<uint32_t*>(MemoryAllocator::FrameAlloc(sizeof(uint32_t) * kMaxEntities, 4));
		//uint32_t* shadowLights = reinterpret_cast<uint32_t*>(MemoryAllocator::FrameAlloc(sizeof(uint32_t) * kMaxShadowCastingLights, 4));

		uint32_t numDirectionalLights = 0;
		uint32_t numPointLights = 0;
		uint32_t numSpotLights = 0;
		uint32_t numShadowCastingLights = 0;
		{
			ambDirLightParams->ambient = math::float4(0.1f, 0.1f, 0.1f, 1.0f);

			for (uint32_t i = 0; i < lightsCount; ++i)
			{
				LightComponentData& comp = lights[i];

				TransformComponent t = comp.entity.GetComponent<TransformComponent>();

				assert(t);

				Transform lightTransform = t->GetWorldTransform();
				math::float3 dir = t->GetForwardVector();

				if (comp.type == kLightTypeDirectional)
				{
					if (numDirectionalLights >= kMaxDirectionalLights)
						continue;

					DirectionalLight& light = ambDirLightParams->directionalLights[numDirectionalLights];
					light.color = comp.lightColor;
					light.direction = math::float4(dir, 0);
					light.intensity = comp.intensity;
					numDirectionalLights++;
				}
				else if (comp.type == kLightTypePoint)
				{
					if (numPointLights >= kMaxPointLights)
						continue;

					lightTransform = 
						Transform(
							math::float3{}, 
							math::quat{}, 
							math::float3(comp.range)) 
						* 
						lightTransform;

					BoundingSphere sphere{};
					sphere.Transform(lightTransform);
					sphere.radius = comp.range;

					if (!camFrustum.Intersects(sphere)) continue;

					auto& light = pointLightParams->lights[numPointLights];
					light.color = comp.lightColor;
					light.intensity = comp.intensity;
					light.range = comp.range;

					pointLightTransform[numPointLights] = math::transpose(lightTransform.GetMatrix());

					numPointLights++;
				}
				else if (comp.type == kLightTypeSpot)
				{
					if (numSpotLights >= kMaxSpotLights)
						continue;

					float tanHalfAngle = math::tan(math::radians(comp.spotAngle * 0.5f));
					float scale = tanHalfAngle * comp.range;
					lightTransform =
						Transform(
							math::float3{},
							math::quat{},
							math::float3(scale, scale, comp.range))
						*
						lightTransform;

					Frustum lightFrustum{
						math::float3{},
						math::quat{},
						tanHalfAngle,
						-tanHalfAngle,
						tanHalfAngle,
						-tanHalfAngle,
						0, comp.range};

					lightFrustum.TransformWithoutScaling(lightTransform);

					if (!camFrustum.Intersects(lightFrustum)) continue;

					auto& light = spotLightParams->lights[numSpotLights];

					light.direction = math::float4(dir, 0);
					light.color = comp.lightColor;
					light.intensity = comp.intensity;
					light.range = comp.range;
					light.spotAngle = math::cos(math::radians(comp.spotAngle * 0.5f));
					light.shadowId = UINT32_MAX;

					spotLightTransform[numSpotLights] = math::transpose(lightTransform.GetMatrix());

					if (comp.castShadow && numShadowCastingLights < kMaxShadowCastingLights)
					{
						math::float3 lightWorldPos = t->GetWorldPosition();

						shadowMatrices[numShadowCastingLights * 4] = math::transpose(
							math::lookTo(lightWorldPos, dir, t->GetUpVector())
						);

						shadowMatrices[numShadowCastingLights * 4 + 1] = math::transpose(
							math::perspective(math::radians(comp.spotAngle), 1.0f, 0.01f, comp.range)
						);

						shadowMatrices[numShadowCastingLights * 4 + 2] =
							shadowMatrices[numShadowCastingLights * 4] *
							shadowMatrices[numShadowCastingLights * 4 + 1];

						light.shadowId = numShadowCastingLights++;
					}

					numSpotLights++;
				}
			}

			pointLightParams->numLights = numPointLights;
			spotLightParams->numLights = numSpotLights;

			ambDirLightParams->ambient.w = float(numDirectionalLights);

			UpdateConstantBuffer(ambientDirLightBuffer, sizeof(LightParametersDirectionalAndAmbient), ambDirLightParams);

			if (numPointLights > 0)
			{
				UpdateConstantBuffer(pointLightBuffer, sizeof(PointLightParams), pointLightParams);
			}

			if (numSpotLights > 0)
			{
				UpdateConstantBuffer(spotLightBuffer, sizeof(SpotLightParams), spotLightParams);
			}

			if (numPointLights > 0)
			{
				UpdateConstantBuffer(pointLightTransformBuffer, sizeof(math::float4x4) * numPointLights, pointLightTransform);
			}

			if (numSpotLights > 0)
			{
				UpdateConstantBuffer(spotLightTransformBuffer, sizeof(math::float4x4) * numSpotLights, spotLightTransform);
			}

			if (numShadowCastingLights > 0)
			{
				UpdateConstantBuffer(shadowMatricesBuffer, sizeof(math::float4x4) * numShadowCastingLights * 4, shadowMatrices);
			}
		}

		// update volumetric fog
		{
			VolumetricFogParams* params = reinterpret_cast<VolumetricFogParams*>(
				MemoryAllocator::FrameAlloc(sizeof(VolumetricFogParams), 16)
				);
			assert(nullptr != params);

			params->fogParams = { 1, 0, 0, 0 };
			params->windDir = math::normalize(math::float3{ 1, 0, 1 }) * 2.0f;
			params->time = Time::TotalTime;
			params->noiseScale = 0.5f;
			params->noiseBase = 0;
			params->noiseAmp = 2;
			params->density = 3;
			params->maxFarClip = 50;
			float fogfarClip = math::min(params->maxFarClip, farClipZ);
			params->maxZ01 = (fogfarClip - nearClipZ) / (farClipZ - nearClipZ);

			UpdateConstantBuffer(fogParamsBuffer, sizeof(VolumetricFogParams), params);
		}

		// update skinned mesh animation bone matrices
		AnimationComponentData* animComps = AnimationComponent::GetAllComponents();
		uint32_t animCompCount = AnimationComponent::GetNumActiveComponents();

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
			void* boneMatrices = MemoryAllocator::FrameAlloc(anim.boneMatricesBufferSize, 4);

			anim.UpdateTiming();
			anim.FillInBoneMatrices(boneMatrices, anim.boneMatricesBufferSize);

			// upload bone matrices
			UpdateConstantBuffer(anim.boneMatricesBuffer, anim.boneMatricesBufferSize, boneMatrices);
		}

		// Shadow Maps

		/*
		for (uint32_t iLight = 0; iLight < numShadowCastingLights; iLight++)
		{
			//uint32_t lightIdx = shadowLights[iLight];

			// clear depth map
			{
				ClearParams* params = MemoryAllocator::FrameAlloc<ClearParams>();
				params->renderTargets[0] = TextureHandle();
				params->depthRenderTarget = shadowMaps[iLight];
				cmdBuf->Add(RendererCommand::kCommandClearRenderTargets, params);
			}

			// TODO: culling
			for (uint32_t iObject = 0; iObject < numOpaqueObjects; iObject++)
			{
				const RenderingObject& obj = activeObjects[iObject];

				assert(obj.model);

				Model& model = models[obj.model.id];
				uint32_t iMesh = obj.submesh;

				assert(model.meshes[iMesh]);
				Mesh& mesh = meshes[model.meshes[iMesh].id];

				const Material* mat = &materials[obj.material.id];

				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();
				params->pipelineState = materialPSOs[mat->type];
				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				if (mat->type == kMaterialDeferredGeometryOpaque) params->pipelineState = materialPSOs[kMaterialShadow];
				if (mat->type == kMaterialDeferredGeometryOpaqueSkinned) params->pipelineState = materialPSOs[kMaterialShadowSkinned];

				switch (mat->type)
				{
				case kMaterialDeferredGeometryOpaqueSkinned:
					(void*)0;
					{
						Entity entity = renderables[obj.renderableId].entity;
						AnimationComponent anim = entity.GetComponent<AnimationComponent>();
						if (!anim || !anim->boneMatricesBuffer)
						{
							return kErrUnknown;
						}
						params->vsConstantBuffers[2] = { anim->boneMatricesBuffer, 0, 0 };
					}
					// fall through
				case kMaterialDeferredGeometryOpaque:
					params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(obj.transformIdx * 16), 16 };
					params->vsConstantBuffers[1] = { shadowMatricesBuffer, static_cast<uint16_t>(iLight * 16), 16 };

					params->renderTargets[0] = TextureHandle();
					params->depthRenderTarget = shadowMaps[iLight];

					break;
				default:
					assert(false && "this material type is not applicable for entities");
					break;
				}

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}
		}
		/**/

		// Geometry Pass

		{
			// clear g-buffer depth
			ClearParams* params = MemoryAllocator::FrameAlloc<ClearParams>();
			params->renderTargets[0] = gBuffer1;
			params->renderTargets[1] = gBuffer2;
			params->renderTargets[2] = gBuffer3;
			params->renderTargets[3] = gBuffer4;
			params->clearColor[3] = 1;
			params->depthRenderTarget = TextureHandle();
			cmdBuf->Add(RendererCommand::kCommandClearRenderTargets, params);
		}

		/*
		// generate draw call for active renderables in command buffer
		for (uint32_t i = 0; i < numOpaqueObjects; ++i)
		{
			RenderingObject& obj = activeObjects[i];

			assert(obj.model);

			Model& model = models[obj.model.id];
			uint32_t iMesh = obj.submesh;

			assert(model.meshes[iMesh]);
			Mesh& mesh = meshes[model.meshes[iMesh].id];

			const Material* mat = &materials[obj.material.id];

			DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();
			params->pipelineState = materialPSOs[mat->type];
			params->vertexBuffer = mesh.VertexBuffer;
			params->indexBuffer = mesh.IndexBuffer;
			params->startIndex = mesh.StartIndex;
			params->startVertex = mesh.StartVertex;
			params->indexCount = mesh.NumIndices;

			//if (mat->type == kMaterialTypeOpaque) params->pipelineState = materialPSOs[kMaterialDeferredGeometryOpaque];
			//if (mat->type == kMaterialTypeOpaqueSkinned) params->pipelineState = materialPSOs[kMaterialDeferredGeometryOpaqueSkinned];

			switch (mat->type)
			{
			case kMaterialDeferredGeometryOpaqueSkinned:
				(void*)0;
				{
					Entity entity = renderables[obj.renderableId].entity;
					AnimationComponent anim = entity.GetComponent<AnimationComponent>();
					if (!anim || !anim->boneMatricesBuffer)
					{
						return kErrUnknown;
					}
					params->vsConstantBuffers[2] = { anim->boneMatricesBuffer, 0, 0 };
				}
				// fall through
			case kMaterialDeferredGeometryOpaque:
				params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(obj.transformIdx * 16), 16 };
				params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 0 };

				params->psConstantBuffers[0] = { mat->materialParamsBuffer, 0, 0 };

				params->psShaderResources[0] = mat->mainTex ? mat->mainTex : defaultAlbedoMap;
				params->psShaderResources[1] = mat->normalMap ? mat->normalMap : defaultNormalMap;
				params->psShaderResources[2] = mat->metallicGlossMap ? mat->metallicGlossMap : defaultMetallicGlossMap;
				params->psShaderResources[3] = mat->occlusionMap ? mat->occlusionMap : defaultOcclusionMap;
				params->psShaderResources[4] = mat->emissionMap ? mat->emissionMap : defaultEmissionMap;
				params->psSamplers[0] = defaultSampler;

				params->renderTargets[0] = gBuffer1;
				params->renderTargets[1] = gBuffer2;
				params->renderTargets[2] = gBuffer3;
				params->renderTargets[3] = gBuffer4;

				break;
			default:
				assert(false && "this material type is not applicable for entities");
				break;
			}

			cmdBuf->Add(RendererCommand::kCommandDraw, params);
		}
		/**/

		// Lighting Pass

		{
			/*
			// occluding

			// go through all point lights
			if (numPointLights > 0)
			{
				Mesh& mesh = meshes[builtinSphere->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();
				assert(nullptr != params);

				params->pipelineState = materialPSOs[kMaterialDeferredLightingOcclude];

				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				params->instanceCount = numPointLights;

				params->vsConstantBuffers[0] = { pointLightTransformBuffer, 0, 0 };
				params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 0 };

				params->renderTargets[0] = TextureHandle();

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}

			// go through all spot lights
			if (numSpotLights > 0)
			{
				Mesh& mesh = meshes[builtinCone->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();
				assert(nullptr != params);

				params->pipelineState = materialPSOs[kMaterialDeferredLightingOcclude];

				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				params->instanceCount = numSpotLights;

				params->vsConstantBuffers[0] = { spotLightTransformBuffer, 0, 0 };
				params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 0 };

				params->renderTargets[0] = TextureHandle();

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}

			// shading

			// go through all point lights
			if (numPointLights > 0)
			{
				Mesh& mesh = meshes[builtinSphere->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();
				assert(nullptr != params);

				params->pipelineState = materialPSOs[kMaterialDeferredLightingPointLight];

				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				params->instanceCount = numPointLights;

				params->vsConstantBuffers[0] = { pointLightTransformBuffer, 0, 0 };
				params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 0 };

				params->psConstantBuffers[0] = params->vsConstantBuffers[0];
				params->psConstantBuffers[1] = params->vsConstantBuffers[1];
				params->psConstantBuffers[2] = { pointLightBuffer, 0, 0 };

				params->psShaderResources[0] = gBuffer1;
				params->psShaderResources[1] = gBuffer2;
				params->psShaderResources[2] = gBuffer3;
				params->psSamplers[0] = defaultSampler;

				params->renderTargets[0] = hdrTarget;

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}

			// go through all spot lights
			if (numSpotLights > 0)
			{
				Mesh& mesh = meshes[builtinCone->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();
				assert(nullptr != params);

				params->pipelineState = materialPSOs[kMaterialDeferredLightingSpotLight];

				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				params->instanceCount = numSpotLights;

				params->vsConstantBuffers[0] = { spotLightTransformBuffer, 0, 0 };
				params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 0 };

				params->psConstantBuffers[0] = params->vsConstantBuffers[0];
				params->psConstantBuffers[1] = params->vsConstantBuffers[1];
				params->psConstantBuffers[2] = { spotLightBuffer, 0, 0 };
				params->psConstantBuffers[3] = { shadowMatricesBuffer, 0, 0 };

				params->psShaderResources[0] = gBuffer1;
				params->psShaderResources[1] = gBuffer2;
				params->psShaderResources[2] = gBuffer3;

				params->psShaderResources[3] = shadowMapArray;

				params->psSamplers[0] = defaultSampler;
				params->psSamplers[1] = shadowSampler;

				params->renderTargets[0] = hdrTarget;

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}
			/**/

			/*
			// ambient light & directional lights
			{
				Mesh& mesh = meshes[builtinQuad->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();

				params->pipelineState = materialPSOs[kMaterialDeferredLightingAmbient];

				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				params->psConstantBuffers[0] = { ambientDirLightBuffer, 0, 0 };
				params->psConstantBuffers[1] = { frameConstantBuffer, 0, 0 };

				params->psShaderResources[0] = gBuffer1;
				params->psShaderResources[1] = gBuffer2;
				params->psShaderResources[2] = gBuffer3;
				params->psShaderResources[3] = gBuffer4;
				if (camera.skybox)
				{
					params->psShaderResources[4] = camera.skyboxDiff;
					params->psShaderResources[5] = camera.skyboxDiff;
					params->psShaderResources[6] = lutMap;
				}

				params->psSamplers[0] = defaultSampler;
				params->psSamplers[1] = lutSampler;

				params->renderTargets[0] = hdrTarget;

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}
			/**/
		}

		/*
		// Transparent Pass
		for (uint32_t i = numOpaqueObjects; i < numActiveObjects; ++i)
		{
			RenderingObject& obj = activeObjects[i];

			assert(obj.model);

			Model& model = models[obj.model.id];
			uint32_t iMesh = obj.submesh;

			assert(model.meshes[iMesh]);
			Mesh& mesh = meshes[model.meshes[iMesh].id];

			const Material* mat = &materials[obj.material.id];

			DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();
			params->pipelineState = materialPSOs[mat->type];
			params->vertexBuffer = mesh.VertexBuffer;
			params->indexBuffer = mesh.IndexBuffer;
			params->startIndex = mesh.StartIndex;
			params->startVertex = mesh.StartVertex;
			params->indexCount = mesh.NumIndices;

			switch (mat->type)
			{
			case kMaterialDeferredTransparentSkinned:
				(void*)0;
				{
					Entity entity = renderables[obj.renderableId].entity;
					AnimationComponent anim = entity.GetComponent<AnimationComponent>();
					if (!anim || !anim->boneMatricesBuffer)
					{
						return kErrUnknown;
					}
					params->vsConstantBuffers[2] = { anim->boneMatricesBuffer, 0, 0 };
				}
				// fall through
			case kMaterialDeferredTransparent:
				params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(obj.transformIdx * 16), 16 };
				params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 0 };
				params->psShaderResources[0] = mat->mainTex ? mat->mainTex : defaultAlbedoMap;
				params->psSamplers[0] = defaultSampler;

				params->renderTargets[0] = hdrTarget;
				break;
			default:
				assert(false && "this material type is not applicable for entities");
				break;
			}

			cmdBuf->Add(RendererCommand::kCommandDraw, params);
		}
		/**/

		// post-process effect
		{

			/*
			{
				ComputeParams* params = MemoryAllocator::FrameAlloc<ComputeParams>();

				params->shader = injectionShader;
				params->constantBuffers[0] = { fogParamsBuffer, 0, 0 };
				params->constantBuffers[1] = { frameConstantBuffer, 0, 0 };
				params->constantBuffers[2] = { spotLightTransformBuffer, 0, 0 };
				params->constantBuffers[3] = { pointLightTransformBuffer, 0, 0 };
				params->constantBuffers[4] = { spotLightBuffer, 0, 0 };
				params->constantBuffers[5] = { pointLightBuffer, 0, 0 };

				params->rwShaderResources[0] = injectionTex;

				params->threadGroupCountX = 10;
				params->threadGroupCountY = 10;
				params->threadGroupCountZ = 32;

				cmdBuf->Add(RendererCommand::kCommandCompute, params);
			}

			{
				ComputeParams* params = MemoryAllocator::FrameAlloc<ComputeParams>();

				params->shader = scatterShader;
				params->shaderResources[0] = injectionTex;
				params->rwShaderResources[0] = scatterTex;

				params->threadGroupCountX = 10;
				params->threadGroupCountY = 10;
				params->threadGroupCountZ = 1;

				cmdBuf->Add(RendererCommand::kCommandCompute, params);
			}

			/**/
			/*

			// Extract bright part
			{
				Mesh& mesh = meshes[builtinQuad->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();

				params->pipelineState = materialPSOs[kMaterialPostProcessExtractBright];

				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				params->psShaderResources[0] = hdrTarget;

				params->renderTargets[0] = hdrTarget2;

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}

			// Blur
			{
				Mesh& mesh = meshes[builtinQuad->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();

				params->pipelineState = materialPSOs[kMaterialPostProcessBlur];

				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				params->psShaderResources[0] = hdrTarget2;

				params->renderTargets[0] = hdrTarget;

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}

			/*

			// volumetric fog
			{
				Mesh& mesh = meshes[builtinQuad->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();

				params->pipelineState = materialPSOs[kMaterialPostProcessVolumetricFog];

				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				params->psConstantBuffers[0] = { fogParamsBuffer, 0, 0 };
				params->psConstantBuffers[1] = { frameConstantBuffer, 0, 0 };

				params->psShaderResources[0] = hdrTarget;
				params->psShaderResources[1] = gBuffer2;
				params->psShaderResources[2] = scatterTex;
				params->psSamplers[0] = defaultSampler;

				params->renderTargets[0] = hdrTarget2;

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}
			/**/

			// tone mapping
			{
				Mesh& mesh = meshes[builtinQuad->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();

				params->pipelineState = materialPSOs[kMaterialPostProcessToneMapping];

				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				params->psShaderResources[0] = hdrTarget;

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}
		}

		return kOK;
	}

	int32_t RenderingSystem::DebugPipeline()
	{
		if (CameraComponent::GetNumActiveComponents() == 0)
		{
			return kOK;
		}
		CameraComponentData& camera = CameraComponent::GetAllComponents()[0];

		// update aspect
		int32_t bufferWidth, bufferHeight;
		{
			renderer->GetFrameBufferSize(bufferWidth, bufferHeight);
			camera.SetAspect(bufferHeight == 0 ? 1.0f : float(bufferWidth) / bufferHeight);
		}

		Frustum camFrustum;
		math::float3 camPos;
		float farClipZ = 0.0f;
		float nearClipZ = 0.0f;
		{
			FrameConstants* data = reinterpret_cast<FrameConstants*>(
				MemoryAllocator::FrameAlloc(sizeof(FrameConstants), 4)
				);
			assert(nullptr != data);

#ifdef TOFU_USE_GLM
			math::float4x4 matView = camera.CalcViewMatrix();
			math::float4x4 matProj = camera.CalcProjectionMatrix();
			data->matView = math::transpose(matView);
			data->matProj = math::transpose(matProj);
			data->matViewInv = math::transpose(math::inverse(matView));
			data->matProjInv = math::transpose(math::inverse(matProj));
#else
			data->matView = camera.CalcViewMatrix();
			data->matProj = camera.CalcProjectionMatrix();
			data->matViewInv = math::inverse(data->matView);
			data->matProjInv = math::inverse(data->matProj);
#endif

			TransformComponent t = camera.entity.GetComponent<TransformComponent>();
			data->cameraPos = math::float4(t->GetWorldPosition(), 1);
			data->bufferSize = math::float4(float(bufferWidth), float(bufferHeight), 0, 0);

			data->perspectiveParams = math::float4{ camera.GetFOV(), camera.GetAspect(), camera.GetZNear(), camera.GetZFar() };
			farClipZ = data->perspectiveParams.w;
			nearClipZ = data->perspectiveParams.z;
			float tanHalfFOV = math::tan(math::radians(data->perspectiveParams.x) * 0.5f);
			float farClipMaxY = tanHalfFOV * farClipZ;
			float farClipMaxX = farClipMaxY * data->perspectiveParams.y;

			const Transform& worldTrans = t->GetWorldTransform();
			data->leftTopRay = worldTrans.TransformVector(math::float4{ -farClipMaxX, farClipMaxY, farClipZ, 0 });
			data->rightTopRay = worldTrans.TransformVector(math::float4{ farClipMaxX, farClipMaxY, farClipZ, 0 });
			data->leftBottomRay = worldTrans.TransformVector(math::float4{ -farClipMaxX, -farClipMaxY, farClipZ, 0 });
			data->rightBottomRay = worldTrans.TransformVector(math::float4{ farClipMaxX, -farClipMaxY, farClipZ, 0 });

			camFrustum.near = nearClipZ;
			camFrustum.far = farClipZ;
			camFrustum.topSlope = tanHalfFOV;
			camFrustum.bottomSlope = -tanHalfFOV;
			camFrustum.rightSlope = tanHalfFOV * data->perspectiveParams.y;
			camFrustum.leftSlope = -camFrustum.rightSlope;
			camFrustum.Transform(t->GetWorldTransform());

			camPos = data->cameraPos;

			UpdateConstantBuffer(frameConstantBuffer, sizeof(FrameConstants), data);
		}

		{
			ClearParams* params = MemoryAllocator::FrameAlloc<ClearParams>();

			params->renderTargets[0] = hdrTarget;
			params->clearColor[0] = 0;
			params->clearColor[1] = 0;
			params->clearColor[2] = 0;
			params->clearColor[3] = 1;

			cmdBuf->Add(RendererCommand::kCommandClearRenderTargets, params);
		}

		for (uint32_t i = 0; i < kMaxMaterials; i++)
		{
			MaterialHandle handle{ i };
			if (materialHandleAlloc.IsValid(handle) && materials[i].isDirty)
			{
				assert(materials[i].materialParamsBuffer);

				void* buffer = MemoryAllocator::FrameAlloc(sizeof(MaterialParams), 4);
				memcpy(buffer, &(materials[i].materialParams), sizeof(MaterialParams));

				UpdateConstantBuffer(materials[i].materialParamsBuffer,
					sizeof(MaterialParams),
					buffer);
				materials[i].isDirty = false;
			}
		}

		// get all renderables in system
		RenderingComponentData* renderables = RenderingComponent::GetAllComponents();
		uint32_t renderableCount = RenderingComponent::GetNumActiveComponents();

		// buffer for transform matrices
		math::float4x4* transformArray = reinterpret_cast<math::float4x4*>(
			MemoryAllocator::FrameAlloc(transformBufferSize, 4)
			);


		// list of active renderables (used for culling)
		RenderingObject* activeObjects = reinterpret_cast<RenderingObject*>(
			MemoryAllocator::FrameAlloc(sizeof(RenderingObject) * kMaxEntities * 2, 4)
			);

		/*uint32_t* opaqueObjects = reinterpret_cast<uint32_t*>(
		MemoryAllocator::FrameAlloc(sizeof(uint32_t) * kMaxEntities, 4)
		);

		uint32_t* transparentObjects = reinterpret_cast<uint32_t*>(
		MemoryAllocator::FrameAlloc(sizeof(uint32_t) * kMaxEntities, 4)
		);*/

		assert(nullptr != transformArray && nullptr != activeObjects);

		uint32_t numActiveEntities = 0;
		uint32_t numActiveObjects = 0;
		uint32_t numOpaqueObjects = 0;
		uint32_t numTransparentObjects = 0;

		// fill in transform matrix for active renderables
		for (uint32_t i = 0; i < renderableCount; ++i)
			// for (uint32_t i = renderableCount - 2; i < renderableCount; ++i)
		{
			RenderingComponentData& comp = renderables[i];

			TransformComponent transform = comp.entity.GetComponent<TransformComponent>();
			assert(transform);

			// TODO culling

			// fill transform matrices;
			uint32_t idx = numActiveEntities++;

#ifdef TOFU_USE_GLM
			transformArray[idx * 4] = math::transpose(transform->GetWorldTransform().GetMatrix());
#else
			transformArrayOpaque[idx * 4] = transform->GetWorldTransform().GetMatrix();
#endif
			transformArray[idx * 4 + 1] = math::transpose(math::inverse(transformArray[idx * 4]));

			// add all submeshes into the queue
			uint32_t numMeshes = comp.model->numMeshes;
			uint32_t numMaterials = comp.numMaterials;
			for (uint32_t j = 0; j < numMeshes; ++j)
			{
				Material* mat = comp.materials[j < numMaterials ? j : (numMaterials - 1)];

				RenderingObject& obj = activeObjects[numActiveObjects++];

				obj.model = comp.model->handle;
				obj.material = mat->handle;
				obj.materialType = mat->type;
				obj.submesh = j;
				obj.renderableId = i;
				obj.transformIdx = idx;

				if (obj.materialType == kMaterialDeferredTransparent || obj.materialType == kMaterialDeferredTransparentSkinned)
				{
					numTransparentObjects++;
				}
				else
				{
					numOpaqueObjects++;
				}
			}
		}

		// sorting rendering objects
		std::sort(activeObjects, activeObjects + numActiveObjects, [](RenderingObject& a, RenderingObject& b)
		{
			return a.materialType < b.materialType;
		});

		// upload transform matrices
		if (numActiveEntities > 0)
		{
			UpdateConstantBuffer(transformBuffer, sizeof(math::float4x4) * 4 * numActiveEntities, transformArray);
		}

		// get all lights
		LightComponentData* lights = LightComponent::GetAllComponents();
		uint32_t lightsCount = LightComponent::GetNumActiveComponents();

		assert(lightsCount <= kMaxEntities);

		LightParametersDirectionalAndAmbient* ambDirLightParams = reinterpret_cast<LightParametersDirectionalAndAmbient*>(
			MemoryAllocator::FrameAlloc(sizeof(LightParametersDirectionalAndAmbient), 16)
			);
		assert(nullptr != ambDirLightParams);

		PointLightParams* pointLightParams = reinterpret_cast<PointLightParams*>(
			MemoryAllocator::FrameAlloc(sizeof(PointLightParams), 16)
			);
		assert(nullptr != pointLightParams);

		SpotLightParams* spotLightParams = reinterpret_cast<SpotLightParams*>(
			MemoryAllocator::FrameAlloc(sizeof(SpotLightParams), 16)
			);
		assert(nullptr != spotLightParams);

		math::float4x4* pointLightTransform = reinterpret_cast<math::float4x4*>(
			MemoryAllocator::FrameAlloc(sizeof(math::float4x4) * kMaxPointLights, 16)
			);
		assert(nullptr != pointLightTransform);

		math::float4x4* spotLightTransform = reinterpret_cast<math::float4x4*>(
			MemoryAllocator::FrameAlloc(sizeof(math::float4x4) * kMaxSpotLights, 16)
			);
		assert(nullptr != spotLightTransform);

		math::float4x4* shadowMatrices = reinterpret_cast<math::float4x4*>(
			MemoryAllocator::FrameAlloc(sizeof(math::float4x4) * kMaxShadowCastingLights * 4, 16)
			);
		assert(nullptr != shadowMatrices);


		//uint32_t* pointLights = reinterpret_cast<uint32_t*>(MemoryAllocator::FrameAlloc(sizeof(uint32_t) * kMaxEntities, 4));
		//uint32_t* spotLights = reinterpret_cast<uint32_t*>(MemoryAllocator::FrameAlloc(sizeof(uint32_t) * kMaxEntities, 4));
		//uint32_t* shadowLights = reinterpret_cast<uint32_t*>(MemoryAllocator::FrameAlloc(sizeof(uint32_t) * kMaxShadowCastingLights, 4));

		uint32_t numDirectionalLights = 0;
		uint32_t numPointLights = 0;
		uint32_t numSpotLights = 0;
		uint32_t numShadowCastingLights = 0;
		{
			ambDirLightParams->ambient = math::float4(0.1f, 0.1f, 0.1f, 1.0f);

			for (uint32_t i = 0; i < lightsCount; ++i)
			{
				LightComponentData& comp = lights[i];

				TransformComponent t = comp.entity.GetComponent<TransformComponent>();

				assert(t);

				Transform lightTransform = t->GetWorldTransform();
				math::float3 dir = t->GetForwardVector();

				if (comp.type == kLightTypeDirectional)
				{
					if (numDirectionalLights >= kMaxDirectionalLights)
						continue;

					DirectionalLight& light = ambDirLightParams->directionalLights[numDirectionalLights];
					light.color = comp.lightColor;
					light.direction = math::float4(dir, 0);
					light.intensity = comp.intensity;
					numDirectionalLights++;
				}
				else if (comp.type == kLightTypePoint)
				{
					if (numPointLights >= kMaxPointLights)
						continue;

					lightTransform =
						Transform(
							math::float3{},
							math::quat{},
							math::float3(comp.range))
						*
						lightTransform;

					BoundingSphere sphere{};
					sphere.Transform(lightTransform);
					sphere.radius = comp.range;

					if (!camFrustum.Intersects(sphere)) continue;

					auto& light = pointLightParams->lights[numPointLights];
					light.color = comp.lightColor;
					light.intensity = comp.intensity;
					light.range = comp.range;

					pointLightTransform[numPointLights] = math::transpose(lightTransform.GetMatrix());

					numPointLights++;
				}
				else if (comp.type == kLightTypeSpot)
				{
					if (numSpotLights >= kMaxSpotLights)
						continue;

					float tanHalfAngle = math::tan(math::radians(comp.spotAngle * 0.5f));
					float scale = tanHalfAngle * comp.range;
					lightTransform =
						Transform(
							math::float3{},
							math::quat{},
							math::float3(scale, scale, comp.range))
						*
						lightTransform;

					Frustum lightFrustum{
						math::float3{},
						math::quat{},
						tanHalfAngle,
						-tanHalfAngle,
						tanHalfAngle,
						-tanHalfAngle,
						0, comp.range };

					lightFrustum.TransformWithoutScaling(lightTransform);

					if (!camFrustum.Intersects(lightFrustum)) continue;

					auto& light = spotLightParams->lights[numSpotLights];

					light.direction = math::float4(dir, 0);
					light.color = comp.lightColor;
					light.intensity = comp.intensity;
					light.range = comp.range;
					light.spotAngle = math::cos(math::radians(comp.spotAngle * 0.5f));
					light.shadowId = UINT32_MAX;

					spotLightTransform[numSpotLights] = math::transpose(lightTransform.GetMatrix());

					if (comp.castShadow && numShadowCastingLights < kMaxShadowCastingLights)
					{
						math::float3 lightWorldPos = t->GetWorldPosition();

						shadowMatrices[numShadowCastingLights * 4] = math::transpose(
							math::lookTo(lightWorldPos, dir, t->GetUpVector())
						);

						shadowMatrices[numShadowCastingLights * 4 + 1] = math::transpose(
							math::perspective(math::radians(comp.spotAngle), 1.0f, 0.01f, comp.range)
						);

						shadowMatrices[numShadowCastingLights * 4 + 2] =
							shadowMatrices[numShadowCastingLights * 4] *
							shadowMatrices[numShadowCastingLights * 4 + 1];

						light.shadowId = numShadowCastingLights++;
					}

					numSpotLights++;
				}
			}

			pointLightParams->numLights = numPointLights;
			spotLightParams->numLights = numSpotLights;

			ambDirLightParams->ambient.w = float(numDirectionalLights);

			UpdateConstantBuffer(ambientDirLightBuffer, sizeof(LightParametersDirectionalAndAmbient), ambDirLightParams);

			if (numPointLights > 0)
			{
				UpdateConstantBuffer(pointLightBuffer, sizeof(PointLightParams), pointLightParams);
			}

			if (numSpotLights > 0)
			{
				UpdateConstantBuffer(spotLightBuffer, sizeof(SpotLightParams), spotLightParams);
			}

			if (numPointLights > 0)
			{
				UpdateConstantBuffer(pointLightTransformBuffer, sizeof(math::float4x4) * numPointLights, pointLightTransform);
			}

			if (numSpotLights > 0)
			{
				UpdateConstantBuffer(spotLightTransformBuffer, sizeof(math::float4x4) * numSpotLights, spotLightTransform);
			}

			if (numShadowCastingLights > 0)
			{
				UpdateConstantBuffer(shadowMatricesBuffer, sizeof(math::float4x4) * numShadowCastingLights * 4, shadowMatrices);
			}
		}

		// update volumetric fog
		{
			VolumetricFogParams* params = reinterpret_cast<VolumetricFogParams*>(
				MemoryAllocator::FrameAlloc(sizeof(VolumetricFogParams), 16)
				);
			assert(nullptr != params);

			params->fogParams = { 1, 0, 0, 0 };
			params->windDir = math::normalize(math::float3{ 1, 0, 1 }) * 2.0f;
			params->time = Time::TotalTime;
			params->noiseScale = 0.5f;
			params->noiseBase = 0;
			params->noiseAmp = 2;
			params->density = 3;
			params->maxFarClip = 50;
			float fogfarClip = math::min(params->maxFarClip, farClipZ);
			params->maxZ01 = (fogfarClip - nearClipZ) / (farClipZ - nearClipZ);

			UpdateConstantBuffer(fogParamsBuffer, sizeof(VolumetricFogParams), params);
		}

		// update skinned mesh animation bone matrices
		AnimationComponentData* animComps = AnimationComponent::GetAllComponents();
		uint32_t animCompCount = AnimationComponent::GetNumActiveComponents();

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
			void* boneMatrices = MemoryAllocator::FrameAlloc(anim.boneMatricesBufferSize, 4);

			anim.UpdateTiming();
			anim.FillInBoneMatrices(boneMatrices, anim.boneMatricesBufferSize);

			// upload bone matrices
			UpdateConstantBuffer(anim.boneMatricesBuffer, anim.boneMatricesBufferSize, boneMatrices);
		}

		// Shadow Maps

		for (uint32_t iLight = 0; iLight < numShadowCastingLights; iLight++)
		{
			//uint32_t lightIdx = shadowLights[iLight];

			// clear depth map
			{
				ClearParams* params = MemoryAllocator::FrameAlloc<ClearParams>();
				params->renderTargets[0] = TextureHandle();
				params->depthRenderTarget = shadowMaps[iLight];
				cmdBuf->Add(RendererCommand::kCommandClearRenderTargets, params);
			}

			// TODO: culling
			for (uint32_t iObject = 0; iObject < numOpaqueObjects; iObject++)
			{
				const RenderingObject& obj = activeObjects[iObject];

				assert(obj.model);

				Model& model = models[obj.model.id];
				uint32_t iMesh = obj.submesh;

				assert(model.meshes[iMesh]);
				Mesh& mesh = meshes[model.meshes[iMesh].id];

				const Material* mat = &materials[obj.material.id];

				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();
				params->pipelineState = materialPSOs[mat->type];
				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				if (mat->type == kMaterialDeferredGeometryOpaque) params->pipelineState = materialPSOs[kMaterialShadow];
				if (mat->type == kMaterialDeferredGeometryOpaqueSkinned) params->pipelineState = materialPSOs[kMaterialShadowSkinned];

				switch (mat->type)
				{
				case kMaterialDeferredGeometryOpaqueSkinned:
					(void*)0;
					{
						Entity entity = renderables[obj.renderableId].entity;
						AnimationComponent anim = entity.GetComponent<AnimationComponent>();
						if (!anim || !anim->boneMatricesBuffer)
						{
							return kErrUnknown;
						}
						params->vsConstantBuffers[2] = { anim->boneMatricesBuffer, 0, 0 };
					}
					// fall through
				case kMaterialDeferredGeometryOpaque:
					params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(obj.transformIdx * 16), 16 };
					params->vsConstantBuffers[1] = { shadowMatricesBuffer, static_cast<uint16_t>(iLight * 16), 16 };

					params->renderTargets[0] = TextureHandle();
					params->depthRenderTarget = shadowMaps[iLight];

					break;
				default:
					assert(false && "this material type is not applicable for entities");
					break;
				}

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}
		}

		// Geometry Pass

		{
			// clear g-buffer depth
			ClearParams* params = MemoryAllocator::FrameAlloc<ClearParams>();
			params->renderTargets[0] = gBuffer1;
			params->renderTargets[1] = gBuffer2;
			params->renderTargets[2] = gBuffer3;
			params->renderTargets[3] = gBuffer4;
			params->clearColor[3] = 1;
			params->depthRenderTarget = TextureHandle();
			cmdBuf->Add(RendererCommand::kCommandClearRenderTargets, params);
		}

		// generate draw call for active renderables in command buffer
		for (uint32_t i = 0; i < numOpaqueObjects; ++i)
		{
			RenderingObject& obj = activeObjects[i];

			assert(obj.model);

			Model& model = models[obj.model.id];
			uint32_t iMesh = obj.submesh;

			assert(model.meshes[iMesh]);
			Mesh& mesh = meshes[model.meshes[iMesh].id];

			const Material* mat = &materials[obj.material.id];

			DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();
			params->pipelineState = materialPSOs[mat->type];
			params->vertexBuffer = mesh.VertexBuffer;
			params->indexBuffer = mesh.IndexBuffer;
			params->startIndex = mesh.StartIndex;
			params->startVertex = mesh.StartVertex;
			params->indexCount = mesh.NumIndices;

			//if (mat->type == kMaterialTypeOpaque) params->pipelineState = materialPSOs[kMaterialDeferredGeometryOpaque];
			//if (mat->type == kMaterialTypeOpaqueSkinned) params->pipelineState = materialPSOs[kMaterialDeferredGeometryOpaqueSkinned];

			switch (mat->type)
			{
			case kMaterialDeferredGeometryOpaqueSkinned:
				(void*)0;
				{
					Entity entity = renderables[obj.renderableId].entity;
					AnimationComponent anim = entity.GetComponent<AnimationComponent>();
					if (!anim || !anim->boneMatricesBuffer)
					{
						return kErrUnknown;
					}
					params->vsConstantBuffers[2] = { anim->boneMatricesBuffer, 0, 0 };
				}
				// fall through
			case kMaterialDeferredGeometryOpaque:
				params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(obj.transformIdx * 16), 16 };
				params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 0 };

				params->psConstantBuffers[0] = { mat->materialParamsBuffer, 0, 0 };

				params->psShaderResources[0] = mat->mainTex ? mat->mainTex : defaultAlbedoMap;
				params->psShaderResources[1] = mat->normalMap ? mat->normalMap : defaultNormalMap;
				params->psShaderResources[2] = mat->metallicGlossMap ? mat->metallicGlossMap : defaultMetallicGlossMap;
				params->psShaderResources[3] = mat->occlusionMap ? mat->occlusionMap : defaultOcclusionMap;
				params->psShaderResources[4] = mat->emissionMap ? mat->emissionMap : defaultEmissionMap;
				params->psSamplers[0] = defaultSampler;

				params->renderTargets[0] = gBuffer1;
				params->renderTargets[1] = gBuffer2;
				params->renderTargets[2] = gBuffer3;
				params->renderTargets[3] = gBuffer4;

				break;
			default:
				assert(false && "this material type is not applicable for entities");
				break;
			}

			cmdBuf->Add(RendererCommand::kCommandDraw, params);
		}

		// Lighting Pass

		{
			// occluding

			// go through all point lights
			if (numPointLights > 0)
			{
				Mesh& mesh = meshes[builtinSphere->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();
				assert(nullptr != params);

				params->pipelineState = materialPSOs[kMaterialDeferredLightingOcclude];

				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				params->instanceCount = numPointLights;

				params->vsConstantBuffers[0] = { pointLightTransformBuffer, 0, 0 };
				params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 0 };

				params->renderTargets[0] = TextureHandle();

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}

			// go through all spot lights
			if (numSpotLights > 0)
			{
				Mesh& mesh = meshes[builtinCone->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();
				assert(nullptr != params);

				params->pipelineState = materialPSOs[kMaterialDeferredLightingOcclude];

				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				params->instanceCount = numSpotLights;

				params->vsConstantBuffers[0] = { spotLightTransformBuffer, 0, 0 };
				params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 0 };

				params->renderTargets[0] = TextureHandle();

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}

			// shading

			// go through all point lights
			if (numPointLights > 0)
			{
				Mesh& mesh = meshes[builtinSphere->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();
				assert(nullptr != params);

				params->pipelineState = materialPSOs[kMaterialDeferredLightingPointLight];

				params->vertexBuffer = mesh.VertexBuffer;
params->indexBuffer = mesh.IndexBuffer;
params->startIndex = mesh.StartIndex;
params->startVertex = mesh.StartVertex;
params->indexCount = mesh.NumIndices;

params->instanceCount = numPointLights;

params->vsConstantBuffers[0] = { pointLightTransformBuffer, 0, 0 };
params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 0 };

params->psConstantBuffers[0] = params->vsConstantBuffers[0];
params->psConstantBuffers[1] = params->vsConstantBuffers[1];
params->psConstantBuffers[2] = { pointLightBuffer, 0, 0 };

params->psShaderResources[0] = gBuffer1;
params->psShaderResources[1] = gBuffer2;
params->psShaderResources[2] = gBuffer3;
params->psSamplers[0] = defaultSampler;

params->renderTargets[0] = hdrTarget;

cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}

			// go through all spot lights
			if (numSpotLights > 0)
			{
				Mesh& mesh = meshes[builtinCone->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();
				assert(nullptr != params);

				params->pipelineState = materialPSOs[kMaterialDeferredLightingSpotLight];

				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				params->instanceCount = numSpotLights;

				params->vsConstantBuffers[0] = { spotLightTransformBuffer, 0, 0 };
				params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 0 };

				params->psConstantBuffers[0] = params->vsConstantBuffers[0];
				params->psConstantBuffers[1] = params->vsConstantBuffers[1];
				params->psConstantBuffers[2] = { spotLightBuffer, 0, 0 };
				params->psConstantBuffers[3] = { shadowMatricesBuffer, 0, 0 };

				params->psShaderResources[0] = gBuffer1;
				params->psShaderResources[1] = gBuffer2;
				params->psShaderResources[2] = gBuffer3;

				params->psShaderResources[3] = shadowMapArray;

				params->psSamplers[0] = defaultSampler;
				params->psSamplers[1] = shadowSampler;

				params->renderTargets[0] = hdrTarget;

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}

			// ambient light & directional lights
			{
				Mesh& mesh = meshes[builtinQuad->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();

				params->pipelineState = materialPSOs[kMaterialDeferredLightingAmbient];

				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				params->psConstantBuffers[0] = { ambientDirLightBuffer, 0, 0 };
				params->psConstantBuffers[1] = { frameConstantBuffer, 0, 0 };

				params->psShaderResources[0] = gBuffer1;
				params->psShaderResources[1] = gBuffer2;
				params->psShaderResources[2] = gBuffer3;
				params->psShaderResources[3] = gBuffer4;
				/*if (camera.skybox)
				{
					params->psShaderResources[4] = camera.skyboxDiff;
					params->psShaderResources[5] = camera.skyboxDiff;
					params->psShaderResources[6] = lutMap;
				}*/

				params->psSamplers[0] = defaultSampler;
				params->psSamplers[1] = lutSampler;

				params->renderTargets[0] = hdrTarget;

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}
		}

		// skybox
		if (camera.skybox)
		{
			Mesh& mesh = meshes[builtinCube->meshes[0].id];

			DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();
			params->pipelineState = materialPSOs[kMaterialSkybox];
			params->vertexBuffer = mesh.VertexBuffer;
			params->indexBuffer = mesh.IndexBuffer;
			params->startIndex = mesh.StartIndex;
			params->startVertex = mesh.StartVertex;
			params->indexCount = mesh.NumIndices;
			params->vsConstantBuffers[0] = { frameConstantBuffer, 0, 0 };

			params->psShaderResources[0] = camera.skybox;
			params->psSamplers[0] = defaultSampler;

			params->renderTargets[0] = hdrTarget;

			cmdBuf->Add(RendererCommand::kCommandDraw, params);
		}

		// Transparent Pass
		for (uint32_t i = numOpaqueObjects; i < numActiveObjects; ++i)
		{
			RenderingObject& obj = activeObjects[i];

			assert(obj.model);

			Model& model = models[obj.model.id];
			uint32_t iMesh = obj.submesh;

			assert(model.meshes[iMesh]);
			Mesh& mesh = meshes[model.meshes[iMesh].id];

			const Material* mat = &materials[obj.material.id];

			DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();
			params->pipelineState = materialPSOs[mat->type];
			params->vertexBuffer = mesh.VertexBuffer;
			params->indexBuffer = mesh.IndexBuffer;
			params->startIndex = mesh.StartIndex;
			params->startVertex = mesh.StartVertex;
			params->indexCount = mesh.NumIndices;

			switch (mat->type)
			{
			case kMaterialDeferredTransparentSkinned:
				(void*)0;
				{
					Entity entity = renderables[obj.renderableId].entity;
					AnimationComponent anim = entity.GetComponent<AnimationComponent>();
					if (!anim || !anim->boneMatricesBuffer)
					{
						return kErrUnknown;
					}
					params->vsConstantBuffers[2] = { anim->boneMatricesBuffer, 0, 0 };
				}
				// fall through
			case kMaterialDeferredTransparent:
				params->vsConstantBuffers[0] = { transformBuffer, static_cast<uint16_t>(obj.transformIdx * 16), 16 };
				params->vsConstantBuffers[1] = { frameConstantBuffer, 0, 0 };
				params->psShaderResources[0] = mat->mainTex ? mat->mainTex : defaultAlbedoMap;
				params->psSamplers[0] = defaultSampler;

				params->renderTargets[0] = hdrTarget;
				break;
			default:
				assert(false && "this material type is not applicable for entities");
				break;
			}

			cmdBuf->Add(RendererCommand::kCommandDraw, params);
		}

		// post-process effect
		{
			{
				ComputeParams* params = MemoryAllocator::FrameAlloc<ComputeParams>();

				params->shader = injectionShader;
				params->constantBuffers[0] = { fogParamsBuffer, 0, 0 };
				params->constantBuffers[1] = { frameConstantBuffer, 0, 0 };
				params->constantBuffers[2] = { spotLightTransformBuffer, 0, 0 };
				params->constantBuffers[3] = { pointLightTransformBuffer, 0, 0 };
				params->constantBuffers[4] = { spotLightBuffer, 0, 0 };
				params->constantBuffers[5] = { pointLightBuffer, 0, 0 };

				params->rwShaderResources[0] = injectionTex;

				params->threadGroupCountX = 10;
				params->threadGroupCountY = 10;
				params->threadGroupCountZ = 32;

				cmdBuf->Add(RendererCommand::kCommandCompute, params);
			}

#if TOFU_FILTER_VFOG == 1
			{
				ComputeParams* params = MemoryAllocator::FrameAlloc<ComputeParams>();

				params->shader = filteringShader;

				params->shaderResources[0] = injectionTex;
				params->rwShaderResources[0] = filteredVolumeTex;

				params->threadGroupCountX = 10;
				params->threadGroupCountY = 10;
				params->threadGroupCountZ = 32;

				cmdBuf->Add(RendererCommand::kCommandCompute, params);
			}
#endif

			{
				ComputeParams* params = MemoryAllocator::FrameAlloc<ComputeParams>();

				params->shader = scatterShader;
#if TOFU_FILTER_VFOG == 1
				params->shaderResources[0] = filteredVolumeTex;
#else
				params->shaderResources[0] = injectionTex;
#endif
				params->rwShaderResources[0] = scatterTex;

				params->threadGroupCountX = 10;
				params->threadGroupCountY = 10;
				params->threadGroupCountZ = 1;

				cmdBuf->Add(RendererCommand::kCommandCompute, params);
			}

			// volumetric fog
			{
				Mesh& mesh = meshes[builtinQuad->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();

				params->pipelineState = materialPSOs[kMaterialPostProcessVolumetricFog];

				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				params->psConstantBuffers[0] = { fogParamsBuffer, 0, 0 };
				params->psConstantBuffers[1] = { frameConstantBuffer, 0, 0 };

				params->psShaderResources[0] = hdrTarget;
				params->psShaderResources[1] = gBuffer2;
				params->psShaderResources[2] = scatterTex;
				params->psSamplers[0] = volumeSampler;

				params->renderTargets[0] = hdrTarget2;

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}

			// tone mapping
			{
				Mesh& mesh = meshes[builtinQuad->meshes[0].id];
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();

				params->pipelineState = materialPSOs[kMaterialPostProcessToneMapping];

				params->vertexBuffer = mesh.VertexBuffer;
				params->indexBuffer = mesh.IndexBuffer;
				params->startIndex = mesh.StartIndex;
				params->startVertex = mesh.StartVertex;
				params->indexCount = mesh.NumIndices;

				params->psShaderResources[0] = hdrTarget2;

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}
		}

		return kOK;
	}
}
