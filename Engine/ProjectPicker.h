#pragma once
#include "ProjectLoader.h"
#include <array>

class ProjectPicker
{
	std::string current_item = "None picked";

	const size_t MAX_PATH_SIZE = 512;
	char* _projPath;

	bool projectPicked = false;
	ProjectLoader& _projectLoader;
	
public:
	ProjectPicker(ProjectLoader& projLoader) : _projectLoader(projLoader)
	{
		_projPath = new char[MAX_PATH_SIZE]();
	}

	~ProjectPicker()
	{
		if (_projPath)
			delete _projPath;
	}

	bool ProjectPicker::Render();
};