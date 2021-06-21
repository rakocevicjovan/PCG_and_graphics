#pragma once

#include "Math.h"
#include "Texture.h"


class BitMapper{

public:
	BitMapper(Texture& tex);
	~BitMapper();

	void init(int smoothness, float widthScale, float lengthScale, float heightScale);
	bool createTerrain();
	bool terrainToFile(const std::string& filePath);

	int granularity;
		
	double xScale, yScale, zScale;

	Texture& _t;

	std::vector<std::vector<std::pair<int, SVec3>>> inVertMap;
	std::vector<std::pair<int, SVec3>> vertexNormals;
	std::vector<std::vector<std::pair<SVec3, SVec3>>> faces; //face indices, face normal vector
		
	std::string inputFileName;
	std::string outputFileName;
};

/*	works, and is optimized... wooooohoo!
BitMapper bitMapper;
bitMapper.init("Textures\\heightMap.png", 1, 1, 1, 0.2);	//g, w, l, h
if (bitMapper.createTerrain())
bitMapper.terrainToFile("Textures\\terrain.obj");
else
std::cout << " Failed to create terrain." << std::endl;
*/