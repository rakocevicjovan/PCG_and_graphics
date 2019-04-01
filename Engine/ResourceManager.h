#pragma once
#include <d3d11.h>
#include <vector>
#include <map>
#include <string>

#include "Texture.h"
#include "CubeMapper.h"
#include "Model.h"
#include "Light.h"
#include "OST.h"
#include "Audio.h"
#include "ParticleSystem.h"

//procedural
#include "Terrain.h"
#include "Perlin.h"
#include "LSystem.h"
#include "Voronoi.h"
#include "Volumetric.h"
#include "BitMapper.h"
#include "Geometry.h"
#include "Maze.h"

class D3D;

struct RenderContext
{
	D3D* d3d;
	float dTime;
	float elapsed;
	ShaderManager* shMan;
	Camera* cam;
};

class Level
{
public:
	virtual void init(ID3D11Device* device) = 0;
	virtual void draw(RenderContext rc) = 0;
};



class OldLevel
{
public:
	void init(ID3D11Device* device) {};
	void draw(ID3D11DeviceContext* deviceContext, RenderContext rc) {};
};



class EarthLevel : public Level
{
public:
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
	void init(ID3D11Device* device);
	void procGen(ID3D11Device* device);
	void draw(RenderContext rc);
};



class FireLevel : public Level
{
public:
	void init(ID3D11Device* device) {};
	void draw(RenderContext rc) {};
};



class WaterLevel : public Level
{
public:
	PointLight pointLight;
	Model skybox;
	CubeMapper skyboxCubeMapper;
	Model will;
	std::map<std::string, Procedural::Terrain> terrainsMap;

	void init(ID3D11Device* device);
	void draw(RenderContext rc) {};
};



class AirLevel : public Level
{
public:
	void init(ID3D11Device* device) {};
	void draw(RenderContext rc) {};
};



class ResourceManager
{

protected:
	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;

public:
	ResourceManager();
	~ResourceManager();

	OldLevel	_level0;
	EarthLevel	_level1;
	FireLevel	_level2;
	WaterLevel	_level3;
	AirLevel	_level4;

	void init(ID3D11Device* device);	//should eventually work from a file
};