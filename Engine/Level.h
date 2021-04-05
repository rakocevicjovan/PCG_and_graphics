#pragma once
#include "Engine.h"
#include "GameObject.h"
#include "Terrain.h"
#include "Perlin.h"
#include "Observer.h"

// For lazy typers (read, me), easy access to engine subsystems and some data
#define S_DEVICE	_sys._D3D.getDevice()
#define S_CONTEXT	_sys._D3D.getContext()
#define S_RANDY		_sys._renderer
#define S_COLLISION	_sys._colEngine
#define S_RESMAN	_sys._resMan
#define S_DOOT		_sys._audio
#define S_INMAN		_sys._inputManager
#define S_SHCACHE	_sys._shaderCache
#define S_MATCACHE	_sys._matCache
#define S_SW		_sys.getScrW()
#define S_SH		_sys.getScrH()
#define S_WW		_sys.getWinW()
#define S_WH		_sys.getWinH()
#define PROJ		_sys.getProject()

class LevelManager;


class Level : public Observer
{
protected:
	Engine& _sys;

	Camera _camera;	// @TODO Remove, add in-game cameras for games and editor camera for editor.

public:
	Level(Engine& sys);
	virtual ~Level() {};

	virtual void init(Engine& sys) = 0;
	virtual void update(const RenderContext& rc) = 0;
	virtual void draw(const RenderContext& rc) = 0;

	bool finished = false;

	virtual void Observe(const MouseClickMessage& c) override {};
	virtual void Observe(const KeyPressMessage& msg) override {};
};