#pragma once
#include "GameObject.h"
#include "Math.h"

class Enemy : public Actor
{
private:

	float _maxHp;
	float _hp = 0;

	float _magicArmour;
	float _physArmour;

public:

	Enemy::Enemy(Model* model, SMatrix& transform = SMatrix(), float maxHp = 100.f, float mArm = 1.f, float pArm = 1.f)
		: Actor(model, transform), _maxHp(maxHp), _magicArmour(mArm), _physArmour(pArm)
	{}

};