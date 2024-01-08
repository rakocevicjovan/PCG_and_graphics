#pragma once

#include "Window.h"
#include "Project.h"
#include "FileBrowser.h"

#include "InputManager.h"
#include "Renderer.h"
#include "GameClock.h"
#include "FPSCounter.h"
#include "Audio.h"
#include "LevelManager.h"

#include "ShaderManager.h"
#include "ShaderCompiler.h"
#include "ShaderCache.h"

#include "multi_threading/VitThreadPool.h"
#include "AssetManagerLocator.h"


#include "Pausable.h"

// Centralized, high level "glue" class that manages engine subsystems and exposes them to the game. Highest scope object
class Engine
{
private:

	bool tick(float dTime);

	UINT _scrWidth;
	UINT _scrHeight;
	UINT _windowWidth = 2560;
	UINT _windowHeight = 1440;

	

	bool _minimized{false};
	//std::unique_ptr<std::thread> _rendering_thread;

	// Contains metadata of the project and lists of assets the project needs
	Project _project;

public:

	Engine();
	~Engine();

	void initialize();
	void start();
	void shutDown();

	// Instead of making the editor right now, I'll just chuck things here and extract them when it makes sense
	FileBrowser _fileBrowser;

	// My very own engine window
	Window<Engine> _engineWindow;
	
	// Engine subsystems
	InputManager _inputManager;
	Audio _audio;
	GameClock _clock;
	FPSCounter _fpsCounter;
	ctpl::thread_pool _threadPool;

	// Rendering
	Renderer _renderer;
	ShaderCompiler _shaderCompiler;
	ShaderCache _shaderCache;

	// Dx11 backend - this should end up behind the renderer, not here
	D3D _D3D;

	// This should be in game code really, it's up to it to define different states etc.
	std::unique_ptr<LevelManager> _levelMan;

	// Managers
	AssetLedger _assetLedger;
	AeonLoader _aeonLoader;

	std::unique_ptr<class ModelManager> _modelManager;
	std::unique_ptr<class SkModelManager> _skModelManager;
	std::unique_ptr<class MaterialManager> _materialManager;
	std::unique_ptr<class TextureManager> _textureManager;
	std::unique_ptr<class SkeletonManager> _skeletonManager;
	std::unique_ptr<class AnimationManager> _animationManager;
	std::unique_ptr<class ShaderManager> _shaderManager;

	AssetManagerLocator _assetManagerLocator;

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