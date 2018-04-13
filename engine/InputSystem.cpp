#include "InputSystem.h"

#include <cassert>
#include <string.h>

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
		states.gamepad.lastState = states.gamepad.state;
		memcpy(states.last_kb_mouse, states.kb_mouse, sizeof(states.kb_mouse));

		NativeContext::instance()->UpdateInputStates(&states);
		return kOK;
	}
}
