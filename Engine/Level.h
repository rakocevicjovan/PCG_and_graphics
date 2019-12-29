#pragma once
#include "Systems.h"
#include "GameObject.h"
#include "Terrain.h"
#include "Perlin.h"
#include "Observer.h"

#define S_DEVICE	_sys._device
#define S_CONTEXT	_sys._deviceContext
#define S_RANDY		_sys._renderer
#define S_SHADY		_sys._renderer._shMan
#define S_COLLISION	_sys._colEngine
#define S_RESMAN	_sys._resMan
#define S_DOOT		_sys._audio
#define S_INMAN		_sys._inputManager
#define S_SHCACHE	_sys._shaderCache
#define S_MATCACHE	_sys._matCache

class LevelManager;

struct GuiElement
{
	std::string title;
	std::string content;
};



class Level : public Observer
{
protected:
	Systems& _sys;

	std::vector<GameObject*> objects;
	std::vector<GraphicComponent*> lesRenderables;
	std::vector<Collider> _levelColliders;

	SVec3 goal;

	Camera _camera;

public:
	Level(Systems& sys);
	virtual ~Level() {};
	
	//@WARNING THIS HAPPENS AUTOMATICALLY IN RENDERER ANYWAYS!!!
	void updateCam(float dTime) { S_RANDY._cam.Update(S_RANDY.rc.dTime); }

	void ProcessSpecialInput(float dTime);

	virtual void init(Systems& sys) = 0;
	virtual void update(const RenderContext& rc) = 0;
	virtual void draw(const RenderContext& rc) = 0;
	virtual void demolish() = 0;
	void win(SVec3 playerPos, float dist = 30.f) { if (SVec3::Distance(playerPos, goal) < dist) finished = true; };
	void procGen() {};

	bool finished = false;

	void renderGuiElems(const std::vector<GuiElement>& elements)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		for (const GuiElement& e : elements)
		{
			ImGui::Begin(e.title.c_str());
			ImGui::Text(e.content.c_str());
			ImGui::End();
		}


		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void Observe(const MouseClickMessage& c) //override
	{

	}
};