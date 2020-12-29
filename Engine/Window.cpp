#include "Window.h"
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <cassert>

// Trick to let enums be used as their underlying type (for bitflags and such)
// can be used internally but clunky to expose to users
namespace
{
	template <typename MyEnumType> 
	inline constexpr std::underlying_type_t<MyEnumType> asUnderlying(MyEnumType flag)
	{
		return static_cast<std::underlying_type_t<MyEnumType>>(flag);
	}
}



void Window::RegisterWindowClass(HINSTANCE hinstance, WNDPROC wndProc, uint32_t flags)
{
	// Setup the windows class with default settings and register it
	WNDCLASSEX wc{};
	wc.style = flags;	//  CS_HREDRAW | CS_VREDRAW | CS_OWNDC
	wc.lpfnWndProc = wndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = CLASS_NAME;
	wc.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);
}


void Window::create(const char* windowName, int w, int h, uint32_t flags)
{
	_hinstance = GetModuleHandle(NULL);	// Get the instance of this application.

	// This is windows specific and shouldn't be exposed in the header. Not sure how other OS do it.
	static std::once_flag window_created{};
	std::call_once(window_created, RegisterWindowClass, _hinstance, WndProc, flags);

	// Creating the actual window. As opposed to class.
	size_t windowNameLength = std::strlen(windowName);
	_windowName = std::make_unique<wchar_t[]>(windowNameLength);
	std::mbstowcs(_windowName.get(), windowName, windowNameLength);

	// Determine the resolution of the clients desktop screen.
	int scrW = GetSystemMetrics(SM_CXSCREEN);
	int scrH = GetSystemMetrics(SM_CYSCREEN);
	
	bool fullScreen = (!w || !h);

	_w = (w) ? scrW : w;
	_h = (h) ? scrH : h;
	_x = (scrW - w) / 2;
	_y = (scrH - h) / 2;

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	DEVMODE dmScreenSettings;
	memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
	dmScreenSettings.dmSize = sizeof(dmScreenSettings);
	dmScreenSettings.dmBitsPerPel = 32;
	dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	dmScreenSettings.dmPelsWidth = (DWORD)scrW;
	dmScreenSettings.dmPelsHeight = (DWORD)scrH;
	if (fullScreen)
	{ 
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	}

	// Create the window with the screen settings and get the handle to it.
	_hwnd = CreateWindowEx(
		WS_EX_APPWINDOW,								// Special property flags (example, taskbar behaviour)
		CLASS_NAME,										// Window class name
		_windowName.get(),								// Name of the window (thanks Cpt. Obvious)
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,	// Style
		_x, _y, _w, _h,									// Can use CW_USEDEFAULT macro as well, but we have our own way
		NULL,											// Parent Window
		NULL,											// Menu
		_hinstance,										// Instance handle. Still not sure what this actually is
		NULL);											// Additional application data. No need for any.

	assert(_hwnd && "Failed to create window.");

	// Bring the window up on the screen and set it as main focus.
	// std::underlying_type<CreationFlags>
	if (flags & CreationFlags::SHOW_WINDOW)
	{
		ShowWindow(_hwnd, SW_SHOW);
	}
	
	if (flags & CreationFlags::START_FOREGROUND)
	{
		SetForegroundWindow(_hwnd);
	}

	if (flags & CreationFlags::START_FOCUSED)
	{
		SetFocus(_hwnd);
	}

	ShowCursor(flags & CreationFlags::SHOW_CURSOR);
}


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, umessage, wparam, lparam))
		return true;

	switch (umessage)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		default:
		{
			return Window::MessageHandler(hwnd, umessage, wparam, lparam);
			//return HandleMessage(hwnd, umessage, wparam, lparam);
		}
	}
}



LRESULT CALLBACK Window::MessageHandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	/*
	switch (message)
	{
	case WM_KEYDOWN:
	{
		_inputManager.setKeyPressed((unsigned int)wparam);
		break;
	}
	case WM_KEYUP:
	{
		_inputManager.setKeyReleased((unsigned int)wparam);
		break;
	}
	case WM_INPUT:
	{
		UINT dwSize;

		GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

		if (dwSize > 0)
		{
			LPBYTE lpb = new BYTE[dwSize];

			if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
				OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				_inputManager.setRelativeXY((short)(raw->data.mouse.lLastX), (short)(raw->data.mouse.lLastY));
			}

			delete[] lpb;
		}
		break;
	}

	//mouse
	{
	case WM_ACTIVATEAPP:
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
		DirectX::Mouse::ProcessMessage(message, wparam, lparam);
		break;
	}
	default:
	{
		return DefWindowProc(hwnd, message, wparam, lparam);
	}
	}
	*/
	return 0;
}