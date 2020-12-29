#pragma once
#include <windows.h>
#include <cstdint>
#include <memory>


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

	static void RegisterWindowClass(HINSTANCE hinstance, WNDPROC wndProc, uint32_t flags);
	static inline const wchar_t* CLASS_NAME = L"AeolianWindowClass";

public:

	// Pass 0 for width and/or height for full screen mode.
	void create(const char* windowName, int w, int h, uint32_t flags);

	struct CreationFlags
	{
		static constexpr uint32_t SHOW_WINDOW = 1 << 0;
		static constexpr uint32_t START_FOREGROUND = 1 << 1;
		static constexpr uint32_t START_FOCUSED = 1 << 2;
		static constexpr uint32_t SHOW_CURSOR = 1 << 3;
	} static constexpr WINDOW_CREATION_FLAGS{};

	static LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);