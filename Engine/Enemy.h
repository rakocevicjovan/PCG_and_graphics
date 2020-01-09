#pragma once
#include "GameObject.h"
#include "Math.h"

struct Armour
{
	float _physArmour;
	float _magicArmour;


	Armour(float phys, float magic) : _physArmour(phys), _magicArmour(magic)
	{}
};



class Enemy : public Actor
{
private:
	float _maxHp;
	float _hp = 0.f;

public:

	Armour _arm;

	Enemy::Enemy(Model* model, SMatrix& transform = SMatrix(), float maxHp = 100.f, float hp = 50.f, float pArm = 1.f, float mArm = 1.f)
		: Actor(model, transform), _maxHp(maxHp), _hp(hp), _arm(pArm, mArm)
	{}

	//void kill() { _alive = false; }
	
	bool isDead()
	{
		return _hp <= 0.f;
	}

	void receiveDamage(float damage)
	{
		_hp -= damage;
	};

	void revive()
	{
		_hp = _maxHp;
	}
};