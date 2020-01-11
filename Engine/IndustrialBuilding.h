#pragma once
#include "Building.h"


class IndustrialBuilding : public Building
{
	Income _income;
	bool _active = true;

public:

	IndustrialBuilding(const Actor& actor, const std::string& name, BuildingType type,
		const BuildingGuiDef& guiDef, const Income& inc)
		: Building(actor, name, type, guiDef), _income(inc)
	{}


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


	virtual IndustrialBuilding* clone() const override
	{
		return new IndustrialBuilding(*this);
	}
};