#pragma once
#include <string>

struct BuildingGuiDef
{
	std::string _desc;
	std::string _name;
	void* _icon;

	BuildingGuiDef() {};

	BuildingGuiDef(const std::string& towerDesc, const std::string& towerName, void* towerIcon)
		: _desc(towerDesc), _name(towerName), _icon(towerIcon) {}
};