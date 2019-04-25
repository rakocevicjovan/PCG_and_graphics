#pragma once
#include "Level.h"
#include "Lillies.h"


class WaterLevel : public Level
{
public:
	WaterLevel(Systems& sys) : Level(sys) {};

	Procedural::Terrain waterTerrain;
	Procedural::LSystem linden;

	std::vector<Collider> _levelColliders;
	Lilly* parentLilly;
	SVec3 oldTranslation;

	Model skybox, modBall, will, lotus, waterSheet, fence, treeModel, lillyModel, lillyPetalModel, throne;
	
	Lillies _lillies;
	Texture lotusTex, waterNormalMap;
	OST reflectionMap, refractionMap;
	SMatrix waterReflectionMatrix;

	PointLight pointLight;
	CubeMapper skyboxCubeMapper, cubeMapper;
	
	void init(Systems& sys);
	void procGen() {};
	void update(const RenderContext& rc);
	void draw(const RenderContext& rc);
	void updateReflectionRefraction(const RenderContext& rc, const Camera& c);
	void setUpCollision();
	void updateCollision();
	void fakeCollision();
	void demolish() { this->~WaterLevel(); };
};
