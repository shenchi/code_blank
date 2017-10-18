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
	constexpr uint32_t MAX_USER_MODULES = 8;
	constexpr uint32_t MAX_ENTITIES = 4096;
	constexpr uint32_t MAX_MESHES = 1024;
}
