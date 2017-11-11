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
		
	public:
		int32_t Init() override
		{
			hInstance = GetModuleHandle(nullptr);

			int32_t w = 1600;// config->GetInt32("display.width", 800);
			int32_t h = 900;// config->GetInt32("display.height", 600);
			std::string title = "tofu";// config->GetString("game.title");
			std::wstring wtitle(title.begin(), title.end());

			WNDCLASSEX cls{ 0 };
			cls.cbSize = sizeof(WNDCLASSEX);
			cls.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
			cls.hInstance = hInstance;
			cls.lpfnWndProc = WndProc;
			cls.lpszClassName = g_WindowClassName;
			cls.style = CS_HREDRAW | CS_VREDRAW;

			if (!RegisterClassEx(&cls))
			{
				return TF_UNKNOWN_ERR;
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
				return TF_UNKNOWN_ERR;
			}

			ShowWindow(hWnd, SW_SHOW);

			keyboard = new DirectX::Keyboard();
			mouse = new DirectX::Mouse();
			mouse->SetWindow(hWnd);
			mouse->SetMode(DirectX::Mouse::Mode::MODE_RELATIVE);
			gamepad = new DirectX::GamePad();

			return TF_OK;
		}

		int32_t Shutdown() override
		{
			delete gamepad;
			delete mouse;
			delete keyboard;

			UnregisterClass(g_WindowClassName, hInstance);
			return TF_OK;
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
			return TF_OK;
		}

		intptr_t GetContextHandle() override
		{
			return reinterpret_cast<intptr_t>(hWnd);
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
				states->kb_mouse[0] |= (1u << TF_MOUSE_LButton);
			}

			if (m.rightButton)
			{
				states->kb_mouse[0] |= (1u << TF_MOUSE_RButton);
			}

			if (m.middleButton)
			{
				states->kb_mouse[0] |= (1u << TF_MOUSE_MButton);
			}
		
			auto pad = gamepad->GetState(0);
			if (pad.IsConnected())
			{

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
