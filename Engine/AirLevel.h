#pragma once
#include "Level.h"



class AirLevel : public Level
{
public:

	Model skybox;
	CubeMapper skyboxCubeMapper;

	Model barrens;

	Texture dragonScales;

	AirLevel(Systems& sys) : Level(sys) {};
	void init(Systems& sys);
	void procGen() {};
	void draw(const RenderContext& rc);
	void demolish() { this->~AirLevel(); };
};
