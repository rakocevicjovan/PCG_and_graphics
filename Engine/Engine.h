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
#include "Project.h"
#include "VitThreadPool.h"
#include "Window.h"


//centralized, high level "glue" class that contains engine subsystems and exposes them to the game, outlives levels
class Engine
{
private:
	bool Frame(float dTime);
	void OutputFPS(float dTime);

	LPCWSTR _applicationName;
	HINSTANCE _hinstance;

	UINT _scrWidth;
	UINT _scrHeight;
	UINT _windowWidth = 1920;
	UINT _windowHeight = 1080;

	// Contains metadata of the project and lists of assets the project needs
	Project _project;

public:
	Engine();
	~Engine();

	bool Initialize();
	void Run();
	void Shutdown();

	// My very own engine window
	Window<Engine> _engineWindow;
	
	// Engine subsystems
	InputManager _inputManager;
	ResourceManager _resMan;
	CollisionEngine _colEngine;
	Audio _audio;
	GameClock _clock;
	ctpl::thread_pool _threadPool;

	// Rendering
	Renderer _renderer;
	ShaderCompiler _shaderCompiler;
	ShaderCache _shaderCache;
	MaterialCache _matCache;

	// Extra rendering data - this should end up in the renderer and loaders ONLY @TODO
	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;
	D3D _D3D;

	// This should be in game code really, it's up to it to define different states etc.
	LevelManager* _levelMan;

	// Here so we have something for camera controls in every level, move out eventually
	Controller _defController;	

	inline UINT getScrW() const { return _scrWidth;  }
	inline UINT getScrH() const { return _scrHeight; }
	inline UINT getWinW() const { return _engineWindow.width();  }
	inline UINT getWinH() const { return _engineWindow.height(); }
	inline const HWND* getHWND() const { return &_engineWindow._hwnd; }
	inline Project& getProject() { return _project; }

	LRESULT HandleWindowInput(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
};