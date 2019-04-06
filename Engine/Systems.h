#pragma once
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "InputManager.h"
#include "Renderer.h"
#include "GameClock.h"
#include "CollisionEngine.h"
#include "Audio.h"
#include "LevelManager.h"
#include "ResourceManager.h"

class Systems
{
public:
	Systems();
	~Systems();

	bool Initialize();
	void Run();
	void Shutdown();
	
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
	
	GameClock gc;
	Renderer _renderer;
	ResourceManager _resMan;
	InputManager _inputManager;
	Audio _audio;
	CollisionEngine _colEngine;
	Controller _controller;
	LevelManager* _levelMan;

	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;

private:
	bool Frame(float dTime);
	void InitializeWindows(int& w, int& h);
	void OutputFPS(float dTime);
	void ProcessInput();

	D3D _D3D;
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;
	POINT midWindow;
	
	int screenWidth, screenHeight, windowWidth, windowHeight;
	float sinceLastInput = 0.f;
};


static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static Systems* ApplicationHandle = 0;