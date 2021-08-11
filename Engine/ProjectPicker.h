#pragma once
#include "Project.h"


class ProjectPicker
{
	std::string _projPath;

	std::unique_ptr<Project> _project;
	
public:

	bool ProjectPicker::Render();
};