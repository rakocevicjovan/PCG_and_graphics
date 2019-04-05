#pragma once
#include "Level.h"


class FireLevel : public Level
{
	Procedural::Terrain terrain, lavaSheet;
	std::vector<Procedural::Terrain> _islands;
	Hexer hexer;
	Model skybox, lavaSheetModel;
	PointLight pointLight;
	CubeMapper skyboxCubeMapper;
	Model will, hexCluster, hexModel;
	Texture hexDiffuseMap, hexNormalMap;
	bool isTerGenerated = false;

public:
	FireLevel(Systems& sys) : Level(sys) {};

	void init(Systems& sys);
	void procGen();
	void draw(const RenderContext& rc);
	void demolish() { this->~FireLevel(); };
};