#pragma once
#include "GameObject.h"

enum class BuildingType { MARTIAL, INDUSTRIAL };


class Building : public Actor
{
	std::string _name;
	BuildingType _type;
};