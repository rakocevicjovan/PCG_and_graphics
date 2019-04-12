#pragma once
#include "Systems.h"
#include "GameObject.h"

#define device		_sys._device
#define context		_sys._deviceContext
#define randy		_sys._renderer
#define collision	_sys.colEngine
#define resources	_sys._resMan
#define doot		_sys._audio
#define inman		_sys._inputManager

class Level
{
protected:
	Systems& _sys;
	float sinceLastInput = 0.f;	//consequence of slightly changing rastertek input instead of completely redoing it... fucking hell

	std::vector<GameObject*> objects;
	std::vector<GraphicComponent*> lesRenderables;
	std::vector<Collider> _levelColliders;

public:
	Level(Systems& sys);
	
	void updateCam(float dTime)
	{
		_sys._renderer._cam.Update(_sys._renderer.rc.dTime);
	}
	void ProcessSpecialInput(float dTime);

	virtual void init(Systems& sys) = 0;
	virtual void draw(const RenderContext& rc) = 0;
	virtual void demolish() = 0;
	virtual void procGen() = 0;
};














class OldLevel : public Level
{
public:
	OldLevel(Systems& sys) : Level(sys) {};

	void init(Systems& sys) {};
	void draw(const RenderContext& rc) {};
	void demolish() {};
	void procGen() {};
};