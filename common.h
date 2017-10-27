#pragma once

#include <cstdint>

#include "Error.h"

#define SINGLETON_DECL(CLASS_NAME) \
	private:\
		static CLASS_NAME* _instance;\
	public:\
		inline static CLASS_NAME* instance() { return _instance; }

#define SINGLETON_IMPL(CLASS_NAME) \
	CLASS_NAME* CLASS_NAME::_instance = nullptr;


#define HANDLE_DECL(CLASS_NAME) \
	struct CLASS_NAME##Handle \
	{ \
		uint32_t id; \
		explicit CLASS_NAME##Handle(uint32_t _id = UINT32_MAX) : id (id) {} \
		operator bool() const { return id != UINT32_MAX; } \
	};

namespace tofu
{
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

	constexpr uint32_t MAX_CONSTANT_BUFFER_BINDINGS = 16;
	constexpr uint32_t MAX_TEXTURE_BINDINGS = 16;
	constexpr uint32_t MAX_SAMPLER_BINDINGS = 16;
	
}
