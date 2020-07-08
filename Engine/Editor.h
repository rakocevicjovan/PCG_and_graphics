#pragma once
#include "FileBrowser.h"
#include "EditorLayout.h"
#include "Math.h"
#include <deque>



class Editor
{
private:

	std::string _projRoot;

	Actor* _selectedActor;

	EditorLayout _layout;

	FileBrowser _fileBrowser;

public:

	Editor() {}



	Editor(float w, float h, const std::string& projRoot) 
		: _layout(w, h), _fileBrowser(projRoot), _projRoot(projRoot), _selectedActor(nullptr)
	{

	}



	void display(std::vector<Actor*>& actors)	//std::vector<GameObject>& objects	//std::vector<Actor*>& actors;
	{
		ImGui::SetNextWindowPos(_layout._assetListPos);
		ImGui::SetNextWindowSize(_layout._assetListSize);

		_fileBrowser.display();

		// List of objects in a level
		ImGui::SetNextWindowPos(_layout._objListPos);
		ImGui::SetNextWindowSize(_layout._objListSize);

		if (ImGui::Begin("Actor list"))
		{
			if (ImGui::BeginCombo("List", "Selected mesh details are shown to the left"))
			{
				for (int i = 0; i < actors.size(); i++)
				{
					std::string actName("Actor nr. " + std::to_string(i));	//actName += std::to_string(i);
					if (ImGui::Selectable(actName.c_str()))	// These guys really need names or something...
					{
						_selectedActor = actors[i];
					}
				}
				ImGui::EndCombo();
			}
		}
		ImGui::End();

		
		// Selected item preview 
		ImGui::SetNextWindowPos(_layout._previewPos);
		ImGui::SetNextWindowSize(_layout._previewSize);

		if (ImGui::Begin("Selected"))
		{
			if (_selectedActor)
			{
				displayActor(*_selectedActor);
			}
		}
		ImGui::End();
	}
};