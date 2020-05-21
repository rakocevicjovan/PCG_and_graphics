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

	int _exporting;

public:

	AssimpLoader(Systems& sys) : Level(sys), _scene(sys, AABB(SVec3(), SVec3(500.f * .5)), 5)
	{
		_browser = FileBrowser("C:\\Users\\Senpai\\source\\repos\\PCG_and_graphics_stale_memes\\Models\\Animated\\Kachujin_walking");
		
_exporting = -1;

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
			if (!alreadyLoaded(selected.value()))
			{
				_previews.push_back(std::make_unique<AssimpPreview>());

				if (!_previews.back()->loadAiScene(rc.d3d->GetDevice(), selected.value().path().string(), 0u))
					_previews.pop_back();
			}
		}



		ImGui::Begin("Content");

		ImGui::BeginTabBar("Loaded scenes");

		ImGui::Separator();
		ImGui::NewLine();

		for (int i = 0; i < _previews.size(); i++)
		{
			std::string sceneName = _previews[i]->getPath().filename().string();
			if (ImGui::BeginTabItem(sceneName.c_str()))
			{
				ImGui::BeginChild(sceneName.c_str(), ImVec2(0., -30.f));

				_previews[i]->displayAiScene(sceneName);	// Add tabs here

				ImGui::EndChild();

				if (ImGui::Button("Export to .aeon"))
				{
					_exporting = i;
				}

				ImGui::EndTabItem();
			}
		}

		ImGui::EndTabBar();

		ImGui::End();

		if (_exporting >= 0)
			exportScene(_previews[_exporting].get());

		GUI::endGuiFrame();

		rc.d3d->EndScene();
	}



	bool alreadyLoaded(const std::filesystem::directory_entry& selected)
	{
		for (auto& p : _previews)
			if (p->getPath() == selected.path())
				return true;

		return false;
	}



	void exportScene(AssimpPreview* preview)
	{
		// Move this to a separate class, it could get big...
		if (ImGui::Begin("Export panel"))
		{
			// Prompt for export options in an easy to use way...

			std::string _exportPath;
			if (inTextStdString("Export path", _exportPath))
			{
				// Check if file already exists to prevent unwanted overwrites
			}

			if (ImGui::Button("Close"))
			{
				_exporting = false;
			}
			
			ImGui::SameLine();
			
			if (ImGui::Button("Commit"))
			{
				//use aeon writer and write to file...
			}
		}
		ImGui::End();
	}
};