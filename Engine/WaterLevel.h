#pragma once
#include "Level.h"
#include "Lillies.h"


class WaterLevel : public Level
{
public:
	WaterLevel(Systems& sys) : Level(sys) {};

	Procedural::Terrain islands, waterTerrain;
	Procedural::LSystem linden;

	std::vector<Collider> _levelColliders;
	Model colModel;

	Model skybox, modBall, will, lotus, waterSheet, fence, lillyModel, treeModel;
	
	Lillies _lillies;
	Texture lotusTex, waterNormalMap;
	OST reflectionMap, refractionMap;
	SMatrix waterReflectionMatrix;

	PointLight pointLight;
	CubeMapper skyboxCubeMapper, cubeMapper;
	
	void init(Systems& sys);
	void procGen() {};
	void draw(const RenderContext& rc);
	void updateReflectionRefraction(const RenderContext& rc, const Camera& c);
	void setUpCollision();
	void updateCollision();
	void demolish() { this->~WaterLevel(); };
};
