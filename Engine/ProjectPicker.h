#pragma once
#include "Project.h"

class ProjectPicker
{
	std::string _currentItemName = "None picked";
	
	std::string _projPath;

	Project* _project;
	
public:

	ProjectPicker() {}

	bool ProjectPicker::Render();
};