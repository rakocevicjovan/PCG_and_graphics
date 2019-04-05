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
#include "Hexer.h"

class D3D;

class ResourceManager
{

public:
	ResourceManager();
	~ResourceManager();
};