#include "pch.h"
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

		GuiBlocks::inputStdString("Choose a project", _projPath);

		if (ImGui::Button("Load project"))
		{ 
			// Check if file exists yada yada
			_project = std::make_unique<Project>();
			if (!_project->loadFromConfig(_projPath))
			{
				_project.reset();
			}
		}

		ImGui::End();
	}

	return done;
}