#pragma once

#include "Common.h"

namespace tofu
{

	enum ButtonId
	{
		TF_KEY_None = 0,

		TF_MOUSE_LButton = 0x1,
		TF_MOUSE_RButton = 0x2,
		TF_MOUSE_MButton = 0x4,

		TF_KEY_Back = 0x8,
		TF_KEY_Tab = 0x9,

		TF_KEY_Enter = 0xd,

		TF_KEY_Pause = 0x13,
		TF_KEY_CapsLock = 0x14,

		TF_KEY_Escape = 0x1b,

		TF_KEY_Space = 0x20,
		TF_KEY_PageUp = 0x21,
		TF_KEY_PageDown = 0x22,
		TF_KEY_End = 0x23,
		TF_KEY_Home = 0x24,
		TF_KEY_Left = 0x25,
		TF_KEY_Up = 0x26,
		TF_KEY_Right = 0x27,
		TF_KEY_Down = 0x28,
		TF_KEY_Select = 0x29,
		TF_KEY_Print = 0x2a,
		TF_KEY_Execute = 0x2b,
		TF_KEY_PrintScreen = 0x2c,
		TF_KEY_Insert = 0x2d,
		TF_KEY_Delete = 0x2e,
		TF_KEY_Help = 0x2f,

		TF_KEY_Alpha0 = 0x30,
		TF_KEY_Alpha1 = 0x31,
		TF_KEY_Alpha2 = 0x32,
		TF_KEY_Alpha3 = 0x33,
		TF_KEY_Alpha4 = 0x34,
		TF_KEY_Alpha5 = 0x35,
		TF_KEY_Alpha6 = 0x36,
		TF_KEY_Alpha7 = 0x37,
		TF_KEY_Alpha8 = 0x38,
		TF_KEY_Alpha9 = 0x39,

		TF_KEY_A = 0x41,
		TF_KEY_B = 0x42,
		TF_KEY_C = 0x43,
		TF_KEY_D = 0x44,
		TF_KEY_E = 0x45,
		TF_KEY_F = 0x46,
		TF_KEY_G = 0x47,
		TF_KEY_H = 0x48,
		TF_KEY_I = 0x49,
		TF_KEY_J = 0x4a,
		TF_KEY_K = 0x4b,
		TF_KEY_L = 0x4c,
		TF_KEY_M = 0x4d,
		TF_KEY_N = 0x4e,
		TF_KEY_O = 0x4f,
		TF_KEY_P = 0x50,
		TF_KEY_Q = 0x51,
		TF_KEY_R = 0x52,
		TF_KEY_S = 0x53,
		TF_KEY_T = 0x54,
		TF_KEY_U = 0x55,
		TF_KEY_V = 0x56,
		TF_KEY_W = 0x57,
		TF_KEY_X = 0x58,
		TF_KEY_Y = 0x59,
		TF_KEY_Z = 0x5a,
		TF_KEY_LeftWindows = 0x5b,
		TF_KEY_RightWindows = 0x5c,
		TF_KEY_Apps = 0x5d,

		TF_KEY_Sleep = 0x5f,
		TF_KEY_NumPad0 = 0x60,
		TF_KEY_NumPad1 = 0x61,
		TF_KEY_NumPad2 = 0x62,
		TF_KEY_NumPad3 = 0x63,
		TF_KEY_NumPad4 = 0x64,
		TF_KEY_NumPad5 = 0x65,
		TF_KEY_NumPad6 = 0x66,
		TF_KEY_NumPad7 = 0x67,
		TF_KEY_NumPad8 = 0x68,
		TF_KEY_NumPad9 = 0x69,
		TF_KEY_Multiply = 0x6a,
		TF_KEY_Add = 0x6b,
		TF_KEY_Separator = 0x6c,
		TF_KEY_Subtract = 0x6d,

		TF_KEY_Decimal = 0x6e,
		TF_KEY_Divide = 0x6f,
		TF_KEY_F1 = 0x70,
		TF_KEY_F2 = 0x71,
		TF_KEY_F3 = 0x72,
		TF_KEY_F4 = 0x73,
		TF_KEY_F5 = 0x74,
		TF_KEY_F6 = 0x75,
		TF_KEY_F7 = 0x76,
		TF_KEY_F8 = 0x77,
		TF_KEY_F9 = 0x78,
		TF_KEY_F10 = 0x79,
		TF_KEY_F11 = 0x7a,
		TF_KEY_F12 = 0x7b,
		TF_KEY_F13 = 0x7c,
		TF_KEY_F14 = 0x7d,
		TF_KEY_F15 = 0x7e,
		TF_KEY_F16 = 0x7f,
		TF_KEY_F17 = 0x80,
		TF_KEY_F18 = 0x81,
		TF_KEY_F19 = 0x82,
		TF_KEY_F20 = 0x83,
		TF_KEY_F21 = 0x84,
		TF_KEY_F22 = 0x85,
		TF_KEY_F23 = 0x86,
		TF_KEY_F24 = 0x87,

		TF_KEY_NumLock = 0x90,
		TF_KEY_Scroll = 0x91,

		TF_KEY_LeftShift = 0xa0,
		TF_KEY_RightShift = 0xa1,
		TF_KEY_LeftControl = 0xa2,
		TF_KEY_RightControl = 0xa3,
		TF_KEY_LeftAlt = 0xa4,
		TF_KEY_RightAlt = 0xa5,

		TF_KEY_MAX_ID = 0xff,

		TF_GAMEPAD_START_ID = 0x100,
		TF_GAMEPAD_FACE_DOWN = 0x100,
		TF_GAMEPAD_FACE_RIGHT = 0x101,
		TF_GAMEPAD_FACE_LEFT = 0x102,
		TF_GAMEPAD_FACE_UP = 0x103,

		TF_GAMEPAD_L3 = 0x104,
		TF_GAMEPAD_R3 = 0x105,
		TF_GAMEPAD_L1 = 0x106,
		TF_GAMEPAD_R1 = 0x107,

		TF_GAMEPAD_BACK = 0x108,
		TF_GAMEPAD_VIEW = 0x108,

		TF_GAMEPAD_START = 0x109,
		TF_GAMEPAD_MENU = 0x109,

		TF_GAMEPAD_DPAD_UP = 0x10a,
		TF_GAMEPAD_DPAD_DOWN = 0x10b,
		TF_GAMEPAD_DPAD_RIGHT = 0x10c,
		TF_GAMEPAD_DPAD_LEFT = 0x10d,

	};

	struct GamePadState
	{
		uint32_t		state;
		float			leftStickX;
		float			leftStickY;
		float			rightStickX;
		float			rightStickY;
		float			leftTrigger;
		float			rightTrigger;
	};

	struct InputStates
	{
		uint32_t		kb_mouse[8];
		float			mouseDeltaX;
		float			mouseDeltaY;
		GamePadState	gamepad;

		inline bool IsButtonDown(uint32_t id) const
		{
			return (kb_mouse[(id >> 5u)] & (1u << (id & 0x1fu))) != 0;
		}
	};

}