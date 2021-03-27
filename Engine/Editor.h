#pragma once
#include "FileBrowser.h"
#include "EditorLayout.h"
#include "Project.h"
#include "Math.h"
#include "SceneEditor.h"
#include "Engine.h"


class Editor
{
private:

	std::unique_ptr<Project> _project;

	FileBrowser _fileBrowser;

	SceneEditor _sceneEditor;

	Engine& _engine;

public:

	Editor(Engine& engine) : _engine(engine)
	{

	}


	void display()
	{
		ImGui::Begin("Editor");

		if (ImGui::BeginMenu("Menu"))
		{
			ImGui::Text("Henlooo, I'm a menu item!");
			ImGui::MenuItem("Me too!", "2");
			ImGui::EndMenu();
		}

		ImGui::End();
	}
};