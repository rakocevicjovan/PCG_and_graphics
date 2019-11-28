#include "ProjectPicker.h"

#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_win32.h"
#include "IMGUI/imgui_impl_dx11.h"



bool ProjectPicker::Render()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	bool retValue = true;

	if (!projectPicked)
	{
		ImGui::Begin("Choose a project to load");
		ImGui::BeginGroup();
		ImGui::Dummy(ImVec2(0.0f, 10.0f));

		if (!_projPath)
			_projPath = new char[MAX_PATH_SIZE]();

		ImGui::InputText("Choose a project:", _projPath, MAX_PATH_SIZE);

		if (ImGui::Button("Load project"))
		{ 
			std::string temp = std::string(_projPath);
			_projectLoader.loadProjFromConfig(temp);
			projectPicked = true;
		}

		ImGui::EndGroup();
	}
	else
	{
		ImGui::Begin("Choose a level to load");
		ImGui::BeginGroup();

		if (ImGui::BeginCombo("Levels associated with the chosen project:", current_item.data()))
		{
			for (int n = 0; n < _projectLoader.getLevelList().size(); ++n)
			{
				bool is_selected = (current_item == _projectLoader.getLevelList()[n]); // You can store your selection however you want, outside or inside your objects
				
				if (ImGui::Selectable(_projectLoader.getLevelList()[n].data(), is_selected))
					current_item = _projectLoader.getLevelList()[n];

					if (is_selected)
						ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}

			ImGui::EndCombo();
		}

		if (ImGui::Button("Load level"))
		{
			delete _projPath;
			_projPath = nullptr;
			retValue = false;
		}

		ImGui::EndGroup();
	}
	
	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return retValue;
}