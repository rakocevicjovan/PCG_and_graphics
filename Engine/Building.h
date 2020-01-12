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


	//return ((this->getPosition() - other.getPosition()).LengthSquared() < 0.1f); - felt like a bad idea, although it does work
	const bool operator==(const Building& other)
	{
		return std::addressof(*this) == std::addressof(other);
	}


	virtual Building* clone() const
	{
		return new Building(*this);
	}
};