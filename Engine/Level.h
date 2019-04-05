#include "Systems.h"



class Level
{
protected:
	Systems* _sys;
	float sinceLastInput = 0.f;

public:
	Level(Systems& sys);
	void ProcessSpecialInput(float dTime);

	virtual void init(Systems& sys) = 0;
	virtual void draw(const RenderContext& rc) = 0;
	virtual void demolish() = 0;
	virtual void procGen() = 0;
};



class OldLevel : public Level
{
public:
	OldLevel(Systems& sys) : Level(sys) {};

	void init(Systems& sys) {};
	void draw(const RenderContext& rc) {};
	void demolish() {};
	void procGen() {};
};



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



class FireLevel : public Level
{
	Procedural::Terrain terrain, island, lavaSheet;
	Hexer hexer;
	Model skybox;
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



class WaterLevel : public Level
{
public:
	WaterLevel(Systems& sys) : Level(sys) {};

	PointLight pointLight;
	Model skybox, modBall;
	CubeMapper skyboxCubeMapper, cubeMapper;
	Model will;
	std::map<std::string, Procedural::Terrain> terrainsMap;

	void init(Systems& sys);
	void procGen() {};
	void draw(const RenderContext& rc);
	void demolish() { this->~WaterLevel(); };
};



class AirLevel : public Level
{
public:
	AirLevel(Systems& sys) : Level(sys) {};
	void init(Systems& sys);
	void procGen() {};
	void draw(const RenderContext& rc);
	void demolish() { this->~AirLevel(); };
};
