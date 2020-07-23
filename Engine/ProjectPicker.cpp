#include "ProjectPicker.h"
#include "GuiBlocks.h"
#include "GUI.h"



bool ProjectPicker::Render()
{
	bool done = true;

	if (!_project)
	{
		ImGui::Begin("Project picker");

		ImGui::NewLine();

		GuiBlocks::inTextStdString("Choose a project", _projPath);

		if (ImGui::Button("Load project"))
		{ 
			// Check if file exists yada yada
			_project = new Project();
			if (!_project->loadFromConfig(_projPath))
			{
				delete _project;
				_project = nullptr;
			}
		}

	}

	ImGui::End();

	return done;
}