#pragma once
#include "Systems.h"
#include "GameObject.h"
#include "Terrain.h"
#include "Perlin.h"

#define device		_sys._device
#define context		_sys._deviceContext
#define randy		_sys._renderer
#define shady		_sys._renderer._shMan
#define collision	_sys._colEngine
#define resources	_sys._resMan
#define doot		_sys._audio
#define inman		_sys._inputManager

class LevelManager;

class Level
{
protected:
	Systems& _sys;

	float sinceLastInput = 0.f;	//consequence of slightly changing rastertek input instead of completely redoing it... fucking hell

	std::vector<GameObject*> objects;
	std::vector<GraphicComponent*> lesRenderables;
	std::vector<Collider> _levelColliders;

	SVec3 goal;

	Camera camera;

public:
	Level(Systems& sys);
	virtual ~Level() {};
	
	void updateCam(float dTime) { randy._cam.Update(randy.rc.dTime); }

	void ProcessSpecialInput(float dTime);

	virtual void init(Systems& sys) = 0;
	virtual void update(const RenderContext& rc) = 0;
	virtual void draw(const RenderContext& rc) = 0;
	virtual void demolish() = 0;
	void win(SVec3 playerPos, float dist = 30.f) { if (SVec3::Distance(playerPos, goal) < dist) finished = true; };
	void procGen() {};

	bool finished = false;

	void renderString(const std::string& title, const std::string& content)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin(title.c_str());
		ImGui::Text(content.c_str());
		ImGui::End();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
};