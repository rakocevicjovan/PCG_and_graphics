#pragma once
#include "Level.h"



class EarthLevel : public Level
{
public:
	EarthLevel(Systems& sys) : Level(sys) {};

	///each level probably contains these
	PointLight pointLight;
	Model skybox;
	CubeMapper skyboxCubeMapper;
	Model  will;
	std::vector<Procedural::Terrain> procTerrains;

	//specific to the level
	Texture mazeDiffuseMap, mazeNormalMap;


	//off-screen render targets
	OST postProcessTexture;	//offScreenTexture
	const unsigned int ostW = 1600, ostH = 900;

	//sounds
	Audio audio;

	//procedural stuff
	Procedural::Terrain proceduralTerrain;
	Procedural::Perlin perlin;
	Procedural::LSystem linden;
	Model treeModel;
	Procedural::Maze maze;

	ParticleSystem pSys;
	std::function<void(ParticleUpdateData*)> particleUpdFunc1;
	std::function<void(ParticleUpdateData*)> particleUpdFunc2;
	bool isTerGenerated = false;

	//load and draw all that jazz
	void init(Systems& sys);
	void procGen();
	void draw(const RenderContext& rc);
	void demolish()
	{
		this->~EarthLevel();
	};
};