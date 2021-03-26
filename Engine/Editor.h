#pragma once
#include "FileBrowser.h"
#include "EditorLayout.h"
#include "Project.h"
#include "Math.h"
#include "SceneEditor.h"


class Editor
{
private:

	Project _project;

	FileBrowser _fileBrowser;

	SceneEditor _sceneEditor;

public:

	Editor() {}

	Editor(float w, float h, Project project) 
		: _project(project), _fileBrowser(project.getProjDir())
	{
		//_sceneEditor.init(scene);
	}


	void display();
};