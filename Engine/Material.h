#pragma once
#include "Texture.h"
#include "ShaderManager.h"



class Shader;
class VertexShader;
class PixelShader;



class Material
{
public:
	//determines whether it goes to the transparent or opaque queue... different sorting
	bool opaque;

	//most important sorting criteria
	VertexShader* vertexShader;
	PixelShader* pixelShader;

	//second most important sorting criteria
	std::vector<Texture*> textures;

	//determines how many textures are added by the shader itself
	//won't ever have 256 textures so that's even too big but can't help it for now without bitsets meh...
	unsigned char texturesAdded = 0u;	

	//don't really sort by these, should be mostly uniform for my uses...
	unsigned int stride = sizeof(Vert3D);
	unsigned int offset = 0u;
	D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	
	Material();
	~Material();

	void setVS(VertexShader* vs) { vertexShader = vs; }
	void setPS(PixelShader* ps) { pixelShader  = ps; }
};