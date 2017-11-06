#pragma once

#include "Common.h"
#include "Module.h"
#include "InputStates.h"

namespace tofu
{
	class InputSystem : public Module
	{
		SINGLETON_DECL(InputSystem)

	public:
		InputSystem();

		virtual int32_t Init() override;

		virtual int32_t Shutdown() override;

		virtual int32_t Update() override;

	public:

		TF_INLINE bool IsButtonDown(ButtonId button) const 
		{ 
			return states.IsButtonDown(button); 
		}

	private:
		InputStates	states;
	};
}
