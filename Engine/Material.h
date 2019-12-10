#pragma once
#include "Texture.h"
#include "ShaderManager.h"


class Shader;


class Material
{
public:
	//int id = 0;
	bool opaque;

	//determines how many textures are added by the shader itself
	//won't ever have 256 textures so that's even too big but can't help it for now without bitsets meh...
	unsigned char texturesAdded = 0u;	

	Shader* vertexShader;
	Shader* pixelShader;

	RenderFormat rFormat;

	std::vector<Texture*> textures;

	Material();
	~Material();

	void setVS(Shader* vs) { vertexShader = vs; }
	void setPS(Shader* ps) { pixelShader  = ps; }
};