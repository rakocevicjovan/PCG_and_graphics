#pragma once
#include <windows.h>
#include <cstdint>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <map>

// Necessary for friend declaration
template <typename WindowInputHandlerType>
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

template <typename WindowInputHandlerType>
class Window
{
private:
	// Position, width, height
	uint16_t _x;
	uint16_t _y;
	uint16_t _w;
	uint16_t _h;

	HWND _hwnd;
	HINSTANCE _hinstance;
	std::unique_ptr<wchar_t[]> _windowName;

	// This is windows specific and shouldn't be exposed by the header. Not sure how other OS do it.
	// @TODO add mapping between my flags and windows flags, for now it's hardcoded
	static void RegisterWindowClass(HINSTANCE hinstance, WNDPROC wndProc, uint32_t flags)
	{
		// Setup the windows class with default settings and register it
		WNDCLASSEX wc{};
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
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

		if (!RegisterClassEx(&wc))
		{
			auto errorCode = GetLastError();
			__debugbreak();
		}
	}
	// I fully expect this to fail on multiple windows being opened. Curently not a concern, fix is easy.
	static inline const wchar_t* CLASS_NAME = L"AeolianWindowClass";

	// Allow access to _hwnd without making it public since we want to conceal Windows specific data types externally 
	friend LRESULT CALLBACK WndProc<WindowInputHandlerType>(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);

	inline static std::map<HWND, WindowInputHandlerType*> WindowInputHandlers;

public:


	// Passing 0 for either width or height results in the window fully covering the given dimension of the screen.
	void createWindow(const char* windowName, WindowInputHandlerType* handler, int w, int h, uint32_t flags)
	{
		_hinstance = GetModuleHandle(NULL);	// Get the instance of this application.

		// Create window class.
		static std::once_flag windowClassCreated{};
		std::call_once(windowClassCreated, RegisterWindowClass, _hinstance, WndProc<WindowInputHandlerType>, flags);

		// Creating the window belonging to the above class.
		size_t windowNameLength = std::strlen(windowName) + 1;
		_windowName = std::make_unique<wchar_t[]>(windowNameLength);
		std::mbstowcs(_windowName.get(), windowName, windowNameLength);

		// Determine the resolution of the clients desktop screen.
		int scrW = GetSystemMetrics(SM_CXSCREEN);
		int scrH = GetSystemMetrics(SM_CYSCREEN);

		_w = (w) ? w : scrW;
		_h = (h) ? h : scrH;
		_x = (scrW - w) / 2;
		_y = (scrH - h) / 2;

		// This should be changed later with a proper flag to support borderless windowed instead of inferring it
		bool fullScreen{ _w == scrW && _h == scrH };

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

		if (!_hwnd)
		{
			auto errCode = GetLastError();
			__debugbreak();
		}

		WindowInputHandlers[_hwnd] = handler;

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


	~Window()
	{
		if (_hwnd && _hinstance)
		{
			destroy();
		}
	}


	void destroy()
	{
		DestroyWindow(_hwnd);
		UnregisterClass(CLASS_NAME, _hinstance);	// @TODO this won't work with multiple wndclass either
		_hwnd = NULL;
		_hinstance = NULL;
	}

	struct CreationFlags
	{
		static constexpr uint32_t SHOW_WINDOW = 1 << 0;
		static constexpr uint32_t START_FOREGROUND = 1 << 1;
		static constexpr uint32_t START_FOCUSED = 1 << 2;
		static constexpr uint32_t SHOW_CURSOR = 1 << 3;
	} static constexpr WINDOW_CREATION_FLAGS{};

	// These two probably shouldn't be here!

	inline UINT width() const { return _w; }
	inline UINT height() const { return _h; }

	// Different type per OS. Think of that when it becomes an issue.
	inline HWND handle() { return _hwnd; }
	inline const HWND* handle() const { return &_hwnd; }
};


template <typename WindowInputHandlerType>
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
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
			return Window<WindowInputHandlerType>::WindowInputHandlers[hwnd]->HandleWindowInput(hwnd, message, wparam, lparam);
		}
	}
}


// Useful misc
namespace
{
	// Trick to let enums be used as their underlying type (for bitflags)
	// Can be used internally but clunky to expose to usersd
	template <typename MyEnumType>
	inline constexpr std::underlying_type_t<MyEnumType> asUnderlying(MyEnumType flag)
	{
		return static_cast<std::underlying_type_t<MyEnumType>>(flag);
	}
}