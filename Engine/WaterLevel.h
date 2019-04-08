#pragma once
#include "Level.h"



class WaterLevel : public Level
{
public:
	WaterLevel(Systems& sys) : Level(sys) {};

	Procedural::Terrain islands, waterTerrain;
	Model skybox, modBall, will, lotus, waterSheet, fence;
	Texture lotusTex, waterNoiseMap;
	OST reflectionMap, refractionMap;
	SPlane clipper;

	PointLight pointLight;
	CubeMapper skyboxCubeMapper, cubeMapper;
	

	void init(Systems& sys);
	void procGen() {};
	void draw(const RenderContext& rc);
	void demolish() { this->~WaterLevel(); };
};
