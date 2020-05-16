#pragma once
#include "Level.h"
#include "AssimpPreview.h"
#include "Systems.h"
#include "Scene.h"
#include "GUI.h"

class AssimpLoader : public Level
{
private:

	Scene _scene;
	AssimpPreview _assimpPreview;

public:

	AssimpLoader(Systems& sys) : Level(sys), _scene(sys, AABB(SVec3(), SVec3(500.f * .5)), 5)
	{
		_assimpPreview.loadAiScene("C:\\Users\\Senpai\\Desktop\\New folder\\ArmyPilot.fbx", 0);
	}


	void init(Systems& sys) override
	{
		
	}



	void update(const RenderContext& rc) override
	{
		
	}



	void draw(const RenderContext& rc) override
	{
		rc.d3d->ClearColourDepthBuffers();

		GUI::startGuiFrame();
		_assimpPreview.displayAiScene();
		GUI::endGuiFrame();

		rc.d3d->EndScene();
	}
};