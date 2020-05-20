#pragma once
#include "Level.h"
#include "AssimpPreview.h"
#include "Systems.h"
#include "Scene.h"
#include "GUI.h"
#include <memory>

class AssimpLoader : public Level
{
private:

	Scene _scene;

	FileBrowser _browser;

	std::vector<std::unique_ptr<AssimpPreview>> _previews;

public:

	AssimpLoader(Systems& sys) : Level(sys), _scene(sys, AABB(SVec3(), SVec3(500.f * .5)), 5)
	{
		_browser = FileBrowser("C:\\Users\\Senpai\\source\\repos\\PCG_and_graphics_stale_memes\\Models\\Animated\\Kachujin_walking");
		
		//_assimpPreview.loadAiScene(sys._device, "C:\\Users\\Senpai\\Desktop\\New folder\\ArmyPilot.fbx", 0);
		//_assimpPreview.loadAiScene(sys._device, "C:\\Users\\Senpai\\source\\repos\\PCG_and_graphics_stale_memes\\Models\\Animated\\Kachujin_walking\\Walking.fbx", 0);
		//_assimpPreview.loadAiScene(sys._device, "C:\\Users\\Senpai\\Desktop\\Erika\\erika_archer_bow_arrow.fbx", 0);
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

		auto selected = _browser.display();

		if (selected.has_value())
		{
			_previews.push_back(std::make_unique<AssimpPreview>());
			if (!_previews.back()->loadAiScene(rc.d3d->GetDevice(), selected.value().path().string(), 0u))
			{
				_previews.pop_back();
			}
		}




		ImGui::Begin("###Loaded scenes");

		ImGui::BeginTabBar("Loaded scenes");

		ImGui::Separator();
		ImGui::NewLine();

		for (int i = 0; i < _previews.size(); i++)
		{
			std::string sceneName = _previews[i]->getPath().filename().string();
			if (ImGui::BeginTabItem(sceneName.c_str()))
			{
				_previews[i]->displayAiScene(sceneName);	// Add tabs here
				ImGui::EndTabItem();
			}
		}

		ImGui::EndTabBar();

		ImGui::End();

		GUI::endGuiFrame();

		rc.d3d->EndScene();
	}
};