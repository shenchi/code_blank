#pragma once

#include <cstdint>

#ifndef _MSC_VER
#include <_types.h>
#endif

#include "Error.h"

#ifndef TF_INLINE
#ifdef _MSC_VER
#define TF_INLINE __forceinline
#else
#define TF_INLINE inline
#endif
#endif

#ifdef _DEBUG
#define CHECKED(x) { int32_t err = kOK; if (kOK != (err = (x))) __debugbreak(); }
#else
#define CHECKED(x) { int32_t err = kOK; if (kOK != (err = (x))) return err; }
#endif

#define SINGLETON_DECL(CLASS_NAME) \
	private:\
		static CLASS_NAME* _instance;\
	public:\
		TF_INLINE static CLASS_NAME* instance() { return _instance; }

#define SINGLETON_IMPL(CLASS_NAME) \
	CLASS_NAME* CLASS_NAME::_instance = nullptr;

struct BaseHandle
{
	union
	{
		struct
		{
			uint32_t		id : 16;
			uint32_t		type : 16;
		};
		uint32_t			raw;
	};

	BaseHandle() : id(UINT16_MAX) {}
	BaseHandle(uint32_t r) : raw(r) {}
	BaseHandle(uint32_t _id, uint32_t _type) : id(_id), type(_type) {}

	TF_INLINE operator bool() const { return id != UINT16_MAX; }
};

#define HANDLE_DECL(CLASS_NAME) \
	struct CLASS_NAME##Handle : BaseHandle \
	{ \
		TF_INLINE explicit CLASS_NAME##Handle() : BaseHandle(((kHandleType##CLASS_NAME) << 16u) | UINT16_MAX) {} \
		TF_INLINE explicit CLASS_NAME##Handle(uint32_t _id) : BaseHandle(_id, kHandleType##CLASS_NAME) {} \
	};

#define TOFU_PERFORMANCE_TIMER_ENABLED 1
#define TOFU_DYNAMIC_INSTANCING_ENABLED 1
#define TOFU_VSYNC 1

namespace tofu
{
	enum HandleType
	{
		kHandleTypeBase,
		kHandleTypeBuffer,
		kHandleTypeTexture,
		kHandleTypeSampler,
		kHandleTypeVertexShader,
		kHandleTypePixelShader,
		kHandleTypeComputeShader,
		kHandleTypePipelineState,
		kHandleTypeMesh,
		kHandleTypeModel,
		kHandleTypeMaterial,
	};

	HANDLE_DECL(Buffer);
	HANDLE_DECL(Texture);
	HANDLE_DECL(Sampler);
	HANDLE_DECL(VertexShader);
	HANDLE_DECL(PixelShader);
	HANDLE_DECL(ComputeShader);
	HANDLE_DECL(PipelineState);
	HANDLE_DECL(Mesh);
	HANDLE_DECL(Model);
	HANDLE_DECL(Material);

	constexpr uint32_t kFrameBufferCount = 2;

	constexpr uint32_t kGlobalMemSize = 128 * 1024 * 1024;
	constexpr uint32_t kGlobalVMemSize = 512 * 1024 * 1024;
	constexpr uint32_t kGlobalMemAlign = 2 * 1024 * 1024;

	constexpr uint32_t kLevelMemSize = 256 * 1024 * 1024;
	constexpr uint32_t kLevelVMemSize = 512 * 1024 * 1024;
	constexpr uint32_t kLevelMemAlign = 2 * 1024 * 1024;

	constexpr uint32_t kFrameMemSize = 512 * 1024 * 1024;
	constexpr uint32_t kFrameVMemSize = 256 * 1024 * 1024;
	constexpr uint32_t kFrameMemAlign = 2 * 1024 * 1024;

	constexpr uint32_t kMaxComponentTypes = 8;
	constexpr uint32_t kMaxUserModules = 8;
	constexpr uint32_t kMaxEntities = 4096;
	constexpr uint32_t kMaxModels = 1024;
	constexpr uint32_t kMaxMeshes = 1024;
	constexpr uint32_t kMaxMaterials = 1024;
	constexpr uint32_t kMaxLights = 255; // deprecated

	constexpr uint32_t kMaxPointLights = 1024;
	constexpr uint32_t kMaxSpotLights = 1024;
	constexpr uint32_t kMaxDirectionalLights = 8;
	constexpr uint32_t kMaxShadowCastingLights = 8;

	constexpr uint32_t kMaxMeshesPerModel = 8;

	constexpr uint32_t kCommandBufferCapacity = 4096;

	constexpr uint32_t kMaxBuffers = 1024;
	constexpr uint32_t kMaxTextures = 1024;
	constexpr uint32_t kMaxSamplers = 256;
	constexpr uint32_t kMaxVertexShaders = 256;
	constexpr uint32_t kMaxPixelShaders = 256;
	constexpr uint32_t kMaxComputeShaders = 256;
	constexpr uint32_t kMaxPipelineStates = 256;

	constexpr uint32_t kMaxConstantBufferBindings = 14;
	constexpr uint32_t kMaxTextureBindings = 16;
	constexpr uint32_t kMaxSamplerBindings = 16;
	constexpr uint32_t kMaxRenderTargetBindings = 8;

	constexpr float kDefaultFixedDeltaTime = 0.0016f;
	constexpr uint32_t kMaxPhysicsStepsPerFrame = 24;
	constexpr uint32_t kMaxLevelResources = 10240;

	constexpr uint32_t kMaxGpuTimeQueries = 8;

	constexpr uint32_t kMaxGUILayers = 8;
}
