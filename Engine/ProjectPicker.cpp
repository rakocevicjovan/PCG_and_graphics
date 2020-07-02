#include "ProjectPicker.h"
#include "GuiBlocks.h"
#include "GUI.h"



bool ProjectPicker::Render()
{
	GUI::beginFrame();

	bool done = true;

	if (!_project)
	{
		ImGui::Begin("Project picker");

		ImGui::NewLine();

		inTextStdString("Choose a project", _projPath);

		if (ImGui::Button("Load project"))
		{ 
			// Check if file exists yada yada
			_project = new Project();
			if (!_project->loadProjFromConfig(_projPath))
			{
				delete _project;
				_project = nullptr;
			}
		}

	}
	else   // This should not be here, project should just open the editor and work from there.
	{
		ImGui::Begin("Choose a level to load");

		auto levelList = _project->getLevelList();

		if (ImGui::BeginCombo("Levels associated with the chosen project:", _currentItemName.data()))
		{
			for (int n = 0; n < levelList.size(); ++n)
			{
				auto level = levelList[n];
				bool is_selected = (_currentItemName == level);
				
				if (ImGui::Selectable(level.data(), is_selected))
					_currentItemName = level;

				if (is_selected)
					ImGui::SetItemDefaultFocus(); 
			}

			ImGui::EndCombo();
		}

		done = !ImGui::Button("Load level");
	}
	ImGui::End();

	GUI::endFrame();

	return done;
}