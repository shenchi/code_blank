#pragma once

#include "Common.h"

namespace tofu
{

	enum ButtonId
	{
		kKeyNone = 0,

		kMouseLButton = 0x1,
		kMouseRButton = 0x2,
		kMouseMButton = 0x4,

		kKeyBack = 0x8,
		kKeyTab = 0x9,

		kKeyEnter = 0xd,

		kKeyPause = 0x13,
		kKeyCapsLock = 0x14,

		kKeyEscape = 0x1b,

		kKeySpace = 0x20,
		kKeyPageUp = 0x21,
		kKeyPageDown = 0x22,
		kKeyEnd = 0x23,
		kKeyHome = 0x24,
		kKeyLeft = 0x25,
		kKeyUp = 0x26,
		kKeyRight = 0x27,
		kKeyDown = 0x28,
		kKeySelect = 0x29,
		kKeyPrint = 0x2a,
		kKeyExecute = 0x2b,
		kKeyPrintScreen = 0x2c,
		kKeyInsert = 0x2d,
		kKeyDelete = 0x2e,
		kKeyHelp = 0x2f,

		kKeyAlpha0 = 0x30,
		kKeyAlpha1 = 0x31,
		kKeyAlpha2 = 0x32,
		kKeyAlpha3 = 0x33,
		kKeyAlpha4 = 0x34,
		kKeyAlpha5 = 0x35,
		kKeyAlpha6 = 0x36,
		kKeyAlpha7 = 0x37,
		kKeyAlpha8 = 0x38,
		kKeyAlpha9 = 0x39,

		kKeyA = 0x41,
		kKeyB = 0x42,
		kKeyC = 0x43,
		kKeyD = 0x44,
		kKeyE = 0x45,
		kKeyF = 0x46,
		kKeyG = 0x47,
		kKeyH = 0x48,
		kKeyI = 0x49,
		kKeyJ = 0x4a,
		kKeyK = 0x4b,
		kKeyL = 0x4c,
		kKeyM = 0x4d,
		kKeyN = 0x4e,
		kKeyO = 0x4f,
		kKeyP = 0x50,
		kKeyQ = 0x51,
		kKeyR = 0x52,
		kKeyS = 0x53,
		kKeyT = 0x54,
		kKeyU = 0x55,
		kKeyV = 0x56,
		kKeyW = 0x57,
		kKeyX = 0x58,
		kKeyY = 0x59,
		kKeyZ = 0x5a,
		kKeyLeftWindows = 0x5b,
		kKeyRightWindows = 0x5c,
		kKeyApps = 0x5d,

		kKeySleep = 0x5f,
		kKeyNumPad0 = 0x60,
		kKeyNumPad1 = 0x61,
		kKeyNumPad2 = 0x62,
		kKeyNumPad3 = 0x63,
		kKeyNumPad4 = 0x64,
		kKeyNumPad5 = 0x65,
		kKeyNumPad6 = 0x66,
		kKeyNumPad7 = 0x67,
		kKeyNumPad8 = 0x68,
		kKeyNumPad9 = 0x69,
		kKeyMultiply = 0x6a,
		kKeyAdd = 0x6b,
		kKeySeparator = 0x6c,
		kKeySubtract = 0x6d,

		kKeyDecimal = 0x6e,
		kKeyDivide = 0x6f,
		kKeyF1 = 0x70,
		kKeyF2 = 0x71,
		kKeyF3 = 0x72,
		kKeyF4 = 0x73,
		kKeyF5 = 0x74,
		kKeyF6 = 0x75,
		kKeyF7 = 0x76,
		kKeyF8 = 0x77,
		kKeyF9 = 0x78,
		kKeyF10 = 0x79,
		kKeyF11 = 0x7a,
		kKeyF12 = 0x7b,
		kKeyF13 = 0x7c,
		kKeyF14 = 0x7d,
		kKeyF15 = 0x7e,
		kKeyF16 = 0x7f,
		kKeyF17 = 0x80,
		kKeyF18 = 0x81,
		kKeyF19 = 0x82,
		kKeyF20 = 0x83,
		kKeyF21 = 0x84,
		kKeyF22 = 0x85,
		kKeyF23 = 0x86,
		kKeyF24 = 0x87,

		kKeyNumLock = 0x90,
		kKeyScroll = 0x91,

		kKeyLeftShift = 0xa0,
		kKeyRightShift = 0xa1,
		kKeyLeftControl = 0xa2,
		kKeyRightControl = 0xa3,
		kKeyLeftAlt = 0xa4,
		kKeyRightAlt = 0xa5,

		kKeyMaxId = 0xff,

		kGamepadStartId = 0x100,

		kGamepadFaceDown = 0x100,
		kGamepadFaceRight = 0x101,
		kGamepadFaceLeft = 0x102,
		kGamepadFaceUp = 0x103,

		kGamepadL3 = 0x104,
		kGamepadR3 = 0x105,
		kGamepadL1 = 0x106,
		kGamepadR1 = 0x107,

		kGamepadBack = 0x108,
		kGamepadView = 0x108,

		kGamepadStart = 0x109,
		kGamepadMenu = 0x109,

		kGamepadDPadUp = 0x10a,
		kGamepadDPadDown = 0x10b,
		kGamepadDPadRight = 0x10c,
		kGamepadDPadLeft = 0x10d,

	};

	struct GamePadState
	{
		uint32_t		state;
		uint32_t		lastState;
		float			leftStickX;
		float			leftStickY;
		float			rightStickX;
		float			rightStickY;
		float			leftTrigger;
		float			rightTrigger;
		bool			isConnected;

		TF_INLINE void SetButtonState(uint32_t id, uint32_t isDown)
		{
			isDown &= 0x1u;

			state |= (isDown << (id - static_cast<uint32_t>(kGamepadStartId)));
			state &= ~((~isDown) << (id - static_cast<uint32_t>(kGamepadStartId)));
		}

		TF_INLINE bool IsButtonDown(uint32_t id) const
		{
			uint32_t mask = (1u << (id - static_cast<uint32_t>(kGamepadStartId)));
			return (state & mask) != 0;
		}

		TF_INLINE bool IsButtonPressed(uint32_t id) const
		{
			uint32_t mask = (1u << (id - static_cast<uint32_t>(kGamepadStartId)));
			return (state & mask) != 0 && (lastState & mask) == 0;
		}

		TF_INLINE bool IsButtonReleased(uint32_t id) const
		{
			uint32_t mask = (1u << (id - static_cast<uint32_t>(kGamepadStartId)));
			return (state & mask) == 0 && (lastState & mask) != 0;
		}
	};

	struct InputStates
	{
		uint32_t		kb_mouse[8];
		uint32_t		last_kb_mouse[8];
		float			mouseDeltaX;
		float			mouseDeltaY;
		GamePadState	gamepad;

		TF_INLINE bool IsButtonDown(uint32_t id) const
		{
			return (kb_mouse[(id >> 5u)] & (1u << (id & 0x1fu))) != 0;
		}

		TF_INLINE bool IsButtonPressed(uint32_t id) const
		{
			return (kb_mouse[(id >> 5u)] & (1u << (id & 0x1fu))) != 0 &&
				(last_kb_mouse[(id >> 5u)] & (1u << (id & 0x1fu))) == 0;
		}

		TF_INLINE bool IsButtonReleased(uint32_t id) const
		{
			return (kb_mouse[(id >> 5u)] & (1u << (id & 0x1fu))) == 0 &&
				(last_kb_mouse[(id >> 5u)] & (1u << (id & 0x1fu))) != 0;
		}
	};

}