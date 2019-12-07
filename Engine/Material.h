#pragma once
#include "Texture.h"
#include "ShaderManager.h"


class Shader;


class Material
{
public:

	int id = 0;
	unsigned char texturesAdded;	//won't ever have over 256 textures so that's even too big but can't help it
	
	Shader* vertexShader;
	Shader* pixelShader;

	RenderFormat rFormat;

	std::vector<Texture*> textures;

	Material();
	~Material();
};

