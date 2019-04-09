#pragma once
#include "Level.h"



class WaterLevel : public Level
{
public:
	WaterLevel(Systems& sys) : Level(sys) {};

	Procedural::Terrain islands, waterTerrain;
	Model skybox, modBall, will, lotus, waterSheet, fence;
	Texture lotusTex, waterNormalMap;
	OST reflectionMap, refractionMap;
	SMatrix waterReflectionMatrix;

	PointLight pointLight;
	CubeMapper skyboxCubeMapper, cubeMapper;
	
	void init(Systems& sys);
	void procGen() {};
	void draw(const RenderContext& rc);
	void updateReflectionRefraction(const RenderContext& rc, const Camera& c);
	void demolish() { this->~WaterLevel(); };
};
