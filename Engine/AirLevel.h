#pragma once
#include "Level.h"
#include "Dragon.h"


class AirLevel : public Level
{
public:

	PointLight pointLight;
	Model skybox;
	Model testBall;
	CubeMapper skyboxCubeMapper;

	Model barrens;

	Dragon dragon;
	Texture dragonScales;
	

	AirLevel(Systems& sys) : Level(sys) {};
	void init(Systems& sys);
	void procGen() {};
	void draw(const RenderContext& rc);
	void demolish() { this->~AirLevel(); };
};
