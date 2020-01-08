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

	Building(const Actor& actor, const std::string& name, BuildingType type, const BuildingGuiDef& guiDef)
		: Actor(actor), _name(name), _type(type), _guiDef(guiDef), _elapsed(0.f) {}

	Building(const Building& b) : Actor(b)
	{
		_name = b._name;
		_type = b._type;
		_elapsed = 0.f;
		_guiDef = b._guiDef;
	}

	virtual Building* clone() const
	{
		return new Building(*this);
	}
};





class MartialBuilding : public Building
{
public:

	MartialBuilding(const Actor& actor, const std::string& name, BuildingType type, 
		const BuildingGuiDef& guiDef, float range, float damage)
		: Building(actor, name, type, guiDef), _range(range), _damage(damage)
	{
		_rangeSq = _range * _range;
	}

	virtual MartialBuilding* clone() const override
	{
		return new MartialBuilding(*this);
	}


	float _range = 30.f;
	float _rangeSq;
	float _damage = 1.f;

	inline bool inRange(const SVec3& enemyPos) const
	{
		return ((enemyPos - getPosition() ).LengthSquared() < _rangeSq);
	}
};





class IndustrialBuilding : public Building
{
	Income _income;
	bool _active = true;

public:

	IndustrialBuilding(const Actor& actor, const std::string& name, BuildingType type,
		const BuildingGuiDef& guiDef, const Income& inc)
		: Building(actor, name, type, guiDef), _income(inc)
	{}

	virtual IndustrialBuilding* clone() const override
	{
		return new IndustrialBuilding(*this);
	}


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