#pragma once
#include "Texture.h"
#include "ShaderManager.h"
#include "Shader.h"

struct RoleTexturePair
{
	TextureRole _role;
	Texture* _tex;
};

//contains everything that the attached shaders need to be set to run... I guess?
class Material
{
protected:

	//most important sorting criteria
	VertexShader* _vertexShader;
	PixelShader* _pixelShader;

public:

	//second most important sorting criteria
	std::vector<RoleTexturePair> _texDescription;
	
	// determines whether it goes to the transparent or opaque queue
	bool _opaque;

	//determines how many textures are added by the shader itself
	//won't ever have 256 textures so that's even too big but can't help it for now without bitsets, meh...
	unsigned char texturesAdded = 0u;

	//don't really sort by these, should be mostly uniform for my uses...
	unsigned int _stride = sizeof(Vert3D);
	unsigned int _offset = 0u;
	D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


	// Functions

	Material();
	Material(VertexShader* vs, PixelShader* ps, bool opaque);
	~Material();
	
	void setVS(VertexShader* vs);
	void setPS(PixelShader* ps);

	void bindTextures(ID3D11DeviceContext* context);

	inline VertexShader* getVS() { return _vertexShader; }
	inline PixelShader* getPS() { return _pixelShader; }
};