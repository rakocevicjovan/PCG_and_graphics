#include "Material.h"



Material::Material() : _vertexShader(nullptr), _pixelShader(nullptr)
{
}


Material::Material(VertexShader* vs, PixelShader* ps, bool opaque) 
	: _vertexShader(vs), _pixelShader(ps), _opaque(opaque) {}


Material::~Material()
{
}



void Material::setVS(VertexShader* vs)
{
	_vertexShader = vs; 
}



void Material::setPS(PixelShader* ps)
{ 
	_pixelShader = ps; 
}



void Material::bindTextures(ID3D11DeviceContext* context)
{
	//TexLayout* tl = _pixelShader->_textureRegisters; use this when you figure out how

	// This does not work as intended yet, the idea is to use _texDescription to set proper fields not naive iteration
	for (int i = 0; i < _texMetaData.size(); ++i)
	{
		context->PSSetShaderResources(_texMetaData[i]._role, 1, &(_texMetaData[i]._tex->_srv));	//texturesAdded + i was used before, deprecated
	}
}