#pragma once
#include "GameObject.h"
#include "BuildingGuiDef.h"
#include "Economy.h"

enum class BuildingType { MARTIAL, INDUSTRIAL };



class Building : public Actor
{
public:
	float _elapsed = 0.f;
	std::string _name;
	BuildingType _type;
	
	BuildingGuiDef _guiDef;

	Building(const Actor& actor, const std::string& name, BuildingType type)
		: Actor(actor), _name(name), _type(type) {}

	//void update(float dTime) { _elapsed += dTime; }
};


class IndustrialBuilding : public Building
{
public:
	Income _income;

	UINT getResources(float dTime)
	{
		_elapsed += dTime;

		if (_elapsed >= _income._tickDuration)
		{
			_elapsed = fmodf(_elapsed, _income._tickDuration);
			return _income._amount;
		}

		return 0u;
	}
};