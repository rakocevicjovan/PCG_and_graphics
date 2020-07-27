#pragma once
#include "Shader.h"
#include <unordered_map>

enum LightModel
{
	NONE,
	LAMBERT,
	PHONG,
	COOK_TORRANCE
};


struct ShaderDescription
{
	uint16_t vsKey;
	uint8_t texRegisters[16];
	unsigned int texUVIndex : 48;
};


struct ShaderPack
{
	VertexShader* vs;
	PixelShader* ps;
};



class ShaderManager
{
	std::map<uint64_t, ShaderPack> _shaderMap;

public:

	

};