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
		: Actor(actor), _name(name), _type(type), _elapsed(0.f) {}

	//void update(float dTime) { _elapsed += dTime; }
};





class MartialBuilding : public Building
{
public:
};





class IndustrialBuilding : public Building
{
	Income _income;
	bool _active = true;

public:

	IndustrialBuilding(const Actor& actor, const std::string& name, BuildingType type, const Income& inc)
		: Building(actor, name, type), _income(inc) {}


	//starts by default, no biggie regardless
	void startUp()
	{
		_active = true;
		_elapsed = 0.f;
	}


	void shutDown()
	{
		_active = false;
		_elapsed = 0.f;
	}

	void getResources(float dTime, std::list<Income>& incomeLedger)
	{
		if (!_active)
			return;

		_elapsed += dTime;

		if (_elapsed >= _income._tickDuration)
		{
			_elapsed = fmodf(_elapsed, _income._tickDuration);
			incomeLedger.push_back(_income);
		}
	}
};