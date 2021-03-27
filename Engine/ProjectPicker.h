#pragma once
#include "Project.h"
#include <memory>

class ProjectPicker
{
	std::string _projPath;

	std::unique_ptr<Project> _project;
	
public:

	bool ProjectPicker::Render();
};