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

