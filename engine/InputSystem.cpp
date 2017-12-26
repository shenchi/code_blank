#include "InputSystem.h"

#include <cassert>

#include "NativeContext.h"

namespace tofu
{
	SINGLETON_IMPL(InputSystem);

	InputSystem::InputSystem()
	{
		assert(nullptr == _instance);
		_instance = this;
	}

	int32_t InputSystem::Init()
	{
		states = {};
		return kOK;
	}

	int32_t InputSystem::Shutdown()
	{
		return kOK;
	}

	int32_t InputSystem::Update()
	{
		NativeContext::instance()->UpdateInputStates(&states);
		return kOK;
	}
}
