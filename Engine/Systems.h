#pragma once
#define WIN32_LEAN_AND_MEAN

#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_win32.h"
#include "IMGUI/imgui_impl_dx11.h"

#include <windows.h>
#include <iostream>
#include <sstream>

#include "InputManager.h"
#include "Renderer.h"
#include "GameClock.h"
#include "CollisionEngine.h"
#include "Audio.h"
#include "LevelManager.h"
#include "ResourceManager.h"
#include "ShaderCompiler.h"
#include "ShaderCache.h"
#include "MaterialCache.h"
#include "VitThreadPool.h"


//centralized, high level "glue" class that contains engine subsystems and exposes them to the game, outlives levels
class Systems
{
private:
	bool Frame(float dTime);
	void InitializeWindows(int& w, int& h);
	void OutputFPS(float dTime);

	LPCWSTR _applicationName;
	HINSTANCE _hinstance;
	HWND _hwnd;
	POINT _midWindow;

	int screenWidth, screenHeight;
	int windowWidth = 1920, windowHeight = 1080;

public:
	Systems();
	~Systems();

	bool Initialize();
	void Run();
	void Shutdown();
	
	InputManager _inputManager;
	ResourceManager _resMan;
	LevelManager* _levelMan;
	CollisionEngine _colEngine;
	Audio _audio;
	GameClock _clock;
	ctpl::thread_pool _threadPool;

	Controller _defController;	// Here so we have something for camera controls in every level, move out eventually

	//rendering
	Renderer _renderer;
	ShaderCompiler _shaderCompiler;
	ShaderCache _shaderCache;
	MaterialCache _matCache;

	//extra rendering data - this should end up in the renderer and loaders ONLY @TODO
	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;
	D3D _D3D;

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

	inline UINT getScrW() const { return screenWidth;  }
	inline UINT getScrH() const { return screenHeight; }
	inline UINT getWinW() const { return windowWidth;  }
	inline UINT getWinH() const { return windowHeight; }

	inline const HWND* getHWND() const { return &_hwnd; }
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static Systems* ApplicationHandle = 0;