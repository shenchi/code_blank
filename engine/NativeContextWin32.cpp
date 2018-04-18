#include "NativeContext.h"

#include "InputStates.h"

#include <Windows.h>
#include <GamePad.h>
#include <Keyboard.h>
#include <Mouse.h>

namespace
{
	wchar_t g_WindowClassName[] = L"TofuNativeWindow";

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_ACTIVATEAPP:
			DirectX::Keyboard::ProcessMessage(msg, wParam, lParam);
			DirectX::Mouse::ProcessMessage(msg, wParam, lParam);
			break;
		case WM_INPUT:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MOUSEHOVER:
			DirectX::Mouse::ProcessMessage(msg, wParam, lParam);
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
			DirectX::Keyboard::ProcessMessage(msg, wParam, lParam);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}

		return 0;
	}
}

namespace tofu
{

	class NativeContextWin32 : public NativeContext
	{
	private:
		HWND					hWnd;
		HINSTANCE				hInstance;

		DirectX::Keyboard*		keyboard;
		DirectX::Mouse*			mouse;
		DirectX::GamePad*		gamepad;

		int32_t					width;
		int32_t					height;
		bool					fullscreen;
		
	public:
		int32_t Init() override
		{
			hInstance = GetModuleHandle(nullptr);

			//int32_t w = 1600;// config->GetInt32("display.width", 800);
			//int32_t h = 900;// config->GetInt32("display.height", 600);
			int32_t w =	1280;
			int32_t h = 720;
			fullscreen = false;

			//int32_t w = 1920;
			//int32_t h = 1080;
			//fullscreen = true;

			std::string title = "tofu";// config->GetString("game.title");
			std::wstring wtitle(title.begin(), title.end());

			WNDCLASSEX cls{ 0 };
			cls.cbSize = sizeof(WNDCLASSEX);
			cls.hInstance = hInstance;
			cls.lpfnWndProc = WndProc;
			cls.lpszClassName = g_WindowClassName;
			cls.style = CS_HREDRAW | CS_VREDRAW;
			if (!fullscreen)
			{
				cls.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
			}

			if (!RegisterClassEx(&cls))
			{
				return kErrUnknown;
			}

			DWORD winStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

			RECT windowRect = { 0, 0, static_cast<LONG>(w), static_cast<LONG>(h) };
			AdjustWindowRect(&windowRect, winStyle, FALSE);

			RECT screenRect;
			GetClientRect(GetDesktopWindow(), &screenRect);
			int windowX = screenRect.right / 2 - (windowRect.right - windowRect.left) / 2;
			int windowY = screenRect.bottom / 2 - (windowRect.bottom - windowRect.top) / 2;

			hWnd = CreateWindow(
				g_WindowClassName,
				wtitle.c_str(),
				winStyle,
				windowX, windowY,
				windowRect.right - windowRect.left,
				windowRect.bottom - windowRect.top,
				nullptr,
				nullptr,
				hInstance,
				nullptr);

			if (!hWnd)
			{
				UnregisterClass(g_WindowClassName, hInstance);
				return kErrUnknown;
			}

			ShowWindow(hWnd, SW_SHOW);

			keyboard = new DirectX::Keyboard();
			mouse = new DirectX::Mouse();
			mouse->SetWindow(hWnd);
			mouse->SetMode(DirectX::Mouse::Mode::MODE_RELATIVE);
			gamepad = new DirectX::GamePad();

			width = uint32_t(w);
			height = uint32_t(h);

			return kOK;
		}

		int32_t Shutdown() override
		{
			delete gamepad;
			delete mouse;
			delete keyboard;

			UnregisterClass(g_WindowClassName, hInstance);
			return kOK;
		}

		bool ProcessEvent() override
		{
			MSG msg;
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					return false;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			return true;
		}

		int32_t QuitApplication() override
		{
			PostQuitMessage(0);
			return kOK;
		}

		intptr_t GetContextHandle() override
		{
			return reinterpret_cast<intptr_t>(hWnd);
		}

		void GetResolution(int32_t* w, int32_t* h) override
		{
			if (fullscreen)
			{
				*w = width;
				*h = height;
			}
			else
			{
				RECT rect = {};
				GetClientRect(hWnd, &rect);
				*w = rect.right - rect.left;
				*h = rect.bottom - rect.top;
			}
		}

		bool IsFullScreen() override
		{
			return fullscreen;
		}

		int64_t GetTimeCounter() override
		{
			int64_t counter;
			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&counter));
			return counter;
		}

		int64_t GetTimeCounterFrequency() override
		{
			int64_t freq;
			QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&freq));
			return freq;
		}

		void UpdateInputStates(InputStates* states) override
		{
			auto kb = keyboard->GetState();
			memcpy(states, &kb, sizeof(kb));

			auto m = mouse->GetState();
			states->mouseDeltaX = static_cast<float>(m.x);
			states->mouseDeltaY = static_cast<float>(m.y);

			if (m.leftButton)
			{
				states->kb_mouse[0] |= (1u << kMouseLButton);
			}

			if (m.rightButton)
			{
				states->kb_mouse[0] |= (1u << kMouseRButton);
			}

			if (m.middleButton)
			{
				states->kb_mouse[0] |= (1u << kMouseMButton);
			}
		
			auto pad = gamepad->GetState(0);
			states->gamepad.isConnected = pad.IsConnected();
			if (pad.IsConnected())
			{
				states->gamepad.SetButtonState(kGamepadFaceDown, pad.buttons.a ? 1u : 0u);
				states->gamepad.SetButtonState(kGamepadFaceRight, pad.buttons.b ? 1u : 0u);
				states->gamepad.SetButtonState(kGamepadFaceLeft, pad.buttons.x ? 1u : 0u);
				states->gamepad.SetButtonState(kGamepadFaceUp, pad.buttons.y ? 1u : 0u);

				states->gamepad.SetButtonState(kGamepadL3, pad.buttons.leftStick ? 1u : 0u);
				states->gamepad.SetButtonState(kGamepadR3, pad.buttons.rightStick ? 1u : 0u);

				states->gamepad.SetButtonState(kGamepadL1, pad.buttons.leftShoulder ? 1u : 0u);
				states->gamepad.SetButtonState(kGamepadR1, pad.buttons.rightShoulder ? 1u : 0u);

				states->gamepad.SetButtonState(kGamepadView, pad.buttons.view ? 1u : 0u);
				states->gamepad.SetButtonState(kGamepadMenu, pad.buttons.menu ? 1u : 0u);

				states->gamepad.SetButtonState(kGamepadDPadUp, pad.dpad.up ? 1u : 0u);
				states->gamepad.SetButtonState(kGamepadDPadDown, pad.dpad.down ? 1u : 0u);
				states->gamepad.SetButtonState(kGamepadDPadRight, pad.dpad.right ? 1u : 0u);
				states->gamepad.SetButtonState(kGamepadDPadLeft, pad.dpad.left ? 1u : 0u);

				states->gamepad.leftStickX = pad.thumbSticks.leftX;
				states->gamepad.leftStickY = pad.thumbSticks.leftY;
				states->gamepad.rightStickX = pad.thumbSticks.rightX;
				states->gamepad.rightStickY = pad.thumbSticks.rightY;
				states->gamepad.leftTrigger = pad.triggers.left;
				states->gamepad.rightTrigger = pad.triggers.right;
			}
		}
	};

	SINGLETON_IMPL(NativeContext);

	NativeContext* NativeContext::Create()
	{
		_instance = new NativeContextWin32();
		return _instance;
	}

}
