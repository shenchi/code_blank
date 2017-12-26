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
			if (button > ButtonId::kKeyMaxId)
			{
				return states.gamepad.IsButtonDown(button);
			} 
			return states.IsButtonDown(button);
		}

		TF_INLINE float GetMouseDeltaX() const
		{
			return states.mouseDeltaX;
		}

		TF_INLINE float GetMouseDeltaY() const
		{
			return states.mouseDeltaY;
		}

		TF_INLINE bool IsGamepadConnected() const
		{
			return states.gamepad.isConnected;
		}

		TF_INLINE float GetLeftStickX() const
		{
			return states.gamepad.leftStickX;
		}

		TF_INLINE float GetLeftStickY() const
		{
			return states.gamepad.leftStickY;
		}

		TF_INLINE float GetRightStickX() const
		{
			return states.gamepad.rightStickX;
		}

		TF_INLINE float GetRightStickY() const
		{
			return states.gamepad.rightStickY;
		}

		TF_INLINE float GetLeftTrigger() const
		{
			return states.gamepad.leftTrigger;
		}

		TF_INLINE float GetRightTrigger() const
		{
			return states.gamepad.rightTrigger;
		}

	private:
		InputStates	states;
	};
}
