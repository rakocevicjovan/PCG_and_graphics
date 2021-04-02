#pragma once
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <iostream>
#include <sstream>

#include "Window.h"
#include "Project.h"
#include "FileBrowser.h"

#include "InputManager.h"
#include "Renderer.h"
#include "GameClock.h"
#include "Audio.h"
#include "LevelManager.h"
#include "ResourceManager.h"
#include "ShaderCompiler.h"
#include "ShaderCache.h"
#include "ShaderManager.h"
#include "MaterialCache.h"
#include "VitThreadPool.h"



// Centralized, high level "glue" class that manages engine subsystems and exposes them to the game. Highest scope object
class Engine
{
private:

	bool tick(float dTime);

	UINT _scrWidth;
	UINT _scrHeight;
	UINT _windowWidth = 1920;
	UINT _windowHeight = 1080;

	// Contains metadata of the project and lists of assets the project needs
	Project _project;

public:

	Engine();
	~Engine();

	bool initialize();
	void start();
	void shutDown();

	// Instead of making the editor right now, I'll just chuck things here and extract them when it makes sense
	FileBrowser _fileBrowser;
	ShaderManager _shaderManager;

	// My very own engine window
	Window<Engine> _engineWindow;
	
	// Engine subsystems
	InputManager _inputManager;
	ResourceManager _resMan;
	Audio _audio;
	GameClock _clock;
	ctpl::thread_pool _threadPool;

	// Rendering
	Renderer _renderer;
	ShaderCompiler _shaderCompiler;
	ShaderCache _shaderCache;
	MaterialCache _matCache;

	// Extra rendering data - this should end up in the renderer and loaders ONLY @TODO
	D3D _D3D;

	// This should be in game code really, it's up to it to define different states etc.
	LevelManager* _levelMan;

	// Here so we have something for camera controls in every level, move out eventually
	Controller _defController;

	inline UINT getScrW() const { return _scrWidth;  }
	inline UINT getScrH() const { return _scrHeight; }
	inline UINT getWinW() const { return _engineWindow.width();  }
	inline UINT getWinH() const { return _engineWindow.height(); }
	inline const HWND* getHWND() const { return _engineWindow.handle(); }
	inline Project& getProject() { return _project; }

	LRESULT HandleWindowInput(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
};