#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "InputManager.h"
#include "Renderer.h"

class AudioEngine;

class Systems{

public:
	Systems();
	~Systems();

	bool Initialize();
	void Run();
	void Shutdown();
	
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int& w, int& h);
	void ShutdownWindows();

private:
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;
	POINT midWindow;

	InputManager _input;
	Renderer* _renderer;
	AudioEngine* _audio;

	int screenWidth, screenHeight, windowWidth, windowHeight;
};


static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static Systems* ApplicationHandle = 0;

#endif