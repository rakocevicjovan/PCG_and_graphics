#pragma once
#include "GameObject.h"
#include "Math.h"

class Enemy : public Actor
{
private:
	//bool _alive;

	float _maxHp;
	float _hp = 0.f;

	float _magicArmour;
	float _physArmour;

public:

	Enemy::Enemy(Model* model, SMatrix& transform = SMatrix(), float maxHp = 100.f, float hp = 50.f, float mArm = 1.f, float pArm = 1.f)
		: Actor(model, transform), _maxHp(maxHp), _hp(hp), _magicArmour(mArm), _physArmour(pArm)
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