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

class Level
{
	virtual void init(ID3D11Device* device) = 0;
	virtual void draw(ID3D11DeviceContext* deviceContext) = 0;
};

class EarthLevel : public Level
{
public:
	//models
	Model modTerrain, modTreehouse, modBall, modSkybox, modWaterQuad, modStrife, modDepths, modBallStand, will;
	std::vector<Model*> _terrainModels;

	//skybox, reflections
	
	CubeMapper cubeMapper, shadowCubeMapper, skyboxCubeMapper;

	//textures
	Texture NST, DST, mazeDiffuseMap, mazeNormalMap;

	//lights
	PointLight pointLight;
	DirectionalLight dirLight;
	
	//off-screen render targets
	OST offScreenTexture, postProcessTexture;
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
	void draw(ID3D11DeviceContext* deviceContext);
	void procGen(ID3D11Device* device);
};



class OldLevel
{
	void init(ID3D11Device* device) {};
	void draw(ID3D11DeviceContext* deviceContext) {};
};



class FireLevel
{
	void init(ID3D11Device* device) {};
	void draw(ID3D11DeviceContext* deviceContext) {};
};



class WaterLevel
{
	void init(ID3D11Device* device) {};
	void draw(ID3D11DeviceContext* deviceContext) {};
};



class AirLevel
{
	void init(ID3D11Device* device) {};
	void draw(ID3D11DeviceContext* deviceContext) {};
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

