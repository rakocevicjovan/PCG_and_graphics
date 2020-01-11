#pragma once
#include "Building.h"
#include "Affliction.h"

class Projectile;

struct Attack
{
	enum class AttackType { PHYS, MAG } _type;

	Projectile* p;

	float _range, _rangeSq;
	float _cooldown;
	float _damage;
	float _flatPen;
	
	Attack(float range = 30.f, float damage = 30.f, AttackType type = AttackType::PHYS, float cooldown = 1.f, float pen = 0.f)
		: _range(range), _damage(damage), _type(type), _cooldown(cooldown), _flatPen(pen)
	{
		_rangeSq = _range * _range;
	}
};



class Tower : public Building
{
protected:

	std::list<Affliction*> _affs;
	Attack _attack;					//could be a pointer but caching... faster in return for some ram, im ok with that

	float _sinceShot = 0.f;

public:

	Tower(const Actor& actor, const std::string& name, BuildingType type,
		const BuildingGuiDef& guiDef, const Attack& a)
		: Building(actor, name, type, guiDef), _attack(a)
	{}



	inline bool inRange(const SVec3& enemyPos) const
	{
		return ((enemyPos - getPosition()).LengthSquared() < _attack._rangeSq);
	}


	//returns the attack type, resets the shot cooldown to full
	inline Attack& shoot()
	{
		_sinceShot = 0.f;
		return _attack;
	}

	
	void advanceCooldown(float dTime)
	{
		_sinceShot += dTime;
	}


	const Attack& getAttack() { return _attack; }


	bool readyToFire() { return _sinceShot >= _attack._cooldown;  }

	virtual Tower* clone() const override
	{
		return new Tower(*this);
	}
};