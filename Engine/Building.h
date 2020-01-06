#pragma once
#include "GameObject.h"
#include "BuildingGuiDef.h"

enum class BuildingType { MARTIAL, INDUSTRIAL };



class Building : public Actor
{
public:
	std::string _name;
	BuildingType _type;
	
	BuildingGuiDef _guiDef;

	Building(const Actor& actor, const std::string& name, BuildingType type)
		: Actor(actor), _name(name), _type(type) {}
};