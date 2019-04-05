#pragma once
#include "Level.h"



class WaterLevel : public Level
{
public:
	WaterLevel(Systems& sys) : Level(sys) {};

	PointLight pointLight;
	Model skybox, modBall;
	CubeMapper skyboxCubeMapper, cubeMapper;
	Model will, lotus;
	Texture lotusTex;

	void init(Systems& sys);
	void procGen() {};
	void draw(const RenderContext& rc);
	void demolish() { this->~WaterLevel(); };
};
