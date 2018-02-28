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

#define CHECKED(x) { int32_t err = kOK; if (kOK != (err = (x))) return err; }
//#define CHECKED(x) { int32_t err = kOK; if (kOK != (err = (x))) __debugbreak(); }

#define SINGLETON_DECL(CLASS_NAME) \
	private:\
		static CLASS_NAME* _instance;\
	public:\
		TF_INLINE static CLASS_NAME* instance() { return _instance; }

#define SINGLETON_IMPL(CLASS_NAME) \
	CLASS_NAME* CLASS_NAME::_instance = nullptr;


#define HANDLE_DECL(CLASS_NAME) \
	struct CLASS_NAME##Handle \
	{ \
		uint32_t id; \
		TF_INLINE explicit CLASS_NAME##Handle(uint32_t _id = UINT32_MAX) : id (_id) {} \
		TF_INLINE operator bool() const { return id != UINT32_MAX; } \
	};


namespace tofu
{
	HANDLE_DECL(Buffer);
	HANDLE_DECL(Texture);
	HANDLE_DECL(Sampler);
	HANDLE_DECL(VertexShader);
	HANDLE_DECL(PixelShader);
	HANDLE_DECL(PipelineState);

	constexpr uint32_t kFrameBufferCount = 2;

	constexpr uint32_t kLevelBasedMemSize = 128 * 1024 * 1024;
	constexpr uint32_t kLevelBasedMemAlign = 2 * 1024 * 1024;

	constexpr uint32_t kFrameBasedMemSize = 128 * 1024 * 1024;
	constexpr uint32_t kFrameBasedMemAlign = 2 * 1024 * 1024;

	constexpr uint32_t kMaxUserModules = 8;
	constexpr uint32_t kMaxEntities = 4096;
	constexpr uint32_t kMaxModels = 1024;
	constexpr uint32_t kMaxMeshes = 1024;
	constexpr uint32_t kMaxMaterials = 1024;
	constexpr uint32_t kMaxLights = 1024;

	constexpr uint32_t kMaxMeshesPerModel = 8;

	constexpr uint32_t kCommandBufferCapacity = 2048;

	constexpr uint32_t kMaxBuffers = 1024;
	constexpr uint32_t kMaxTextures = 1024;
	constexpr uint32_t kMaxSamplers = 256;
	constexpr uint32_t kMaxVertexShaders = 256;
	constexpr uint32_t kMaxPixelShaders = 256;
	constexpr uint32_t kMaxPipelineStates = 256;

	constexpr uint32_t kMaxConstantBufferBindings = 14;
	constexpr uint32_t kMaxTextureBindings = 16;
	constexpr uint32_t kMaxSamplerBindings = 16;
	constexpr uint32_t kMaxRenderTargetBindings = 8;
	
}
