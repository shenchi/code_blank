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

	constexpr uint32_t FRAME_BUFFER_COUNT = 2;

	constexpr uint32_t LEVEL_BASED_MEM_SIZE = 128 * 1024 * 1024;
	constexpr uint32_t LEVEL_BASED_MEM_ALIGN = 2 * 1024 * 1024;

	constexpr uint32_t FRAME_BASED_MEM_SIZE = 128 * 1024 * 1024;
	constexpr uint32_t FRAME_BASED_MEM_ALIGN = 2 * 1024 * 1024;

	constexpr uint32_t MAX_USER_MODULES = 8;
	constexpr uint32_t MAX_ENTITIES = 4096;
	constexpr uint32_t MAX_MODELS = 1024;
	constexpr uint32_t MAX_MESHES = 1024;
	constexpr uint32_t MAX_MATERIALS = 1024;

	constexpr uint32_t MAX_MESHES_PER_MODEL = 8;

	constexpr uint32_t COMMAND_BUFFER_CAPACITY = 64;

	constexpr uint32_t MAX_BUFFERS = 1024;
	constexpr uint32_t MAX_TEXTURES = 1024;
	constexpr uint32_t MAX_SAMPLERS = 256;
	constexpr uint32_t MAX_VERTEX_SHADERS = 256;
	constexpr uint32_t MAX_PIXEL_SHADERS = 256;
	constexpr uint32_t MAX_PIPELINE_STATES = 256;

	constexpr uint32_t MAX_CONSTANT_BUFFER_BINDINGS = 14;
	constexpr uint32_t MAX_TEXTURE_BINDINGS = 16;
	constexpr uint32_t MAX_SAMPLER_BINDINGS = 16;
	constexpr uint32_t MAX_RENDER_TARGET_BINDINGS = 8;
	
}
