#pragma once
#include <d3d11.h>
#include <vector>
#include <map>
#include <string>

#include "Texture.h"
#include "Model.h"
#include "Light.h"
#include "OST.h"
#include "Audio.h"

struct Level
{
	//models
	Model modTerrain, modTreehouse, modBall, modSkybox, modWaterQuad, modStrife, modDepths, modBallStand, will;
	std::vector<Model*> _terrainModels;
	
	//materials
	std::map <std::string, Material> materials;

	//textures
	Texture NST, DST, white, perlinTex, worley;

	//lights
	PointLight pointLight;
	DirectionalLight dirLight;
	
	//off-screen render targets
	OST offScreenTexture, postProcessTexture;
	const unsigned int ostW = 1600, ostH = 900;

	//sounds
	Audio audio;


	//load all that jazz
	void init(ID3D11Device* device);
};



class ResourceManager
{
protected:
	
	ID3D11Device* _device;

public:
	ResourceManager();
	~ResourceManager();

	Level _level;

	void init(ID3D11Device* device);	//should eventually work from a file
};

