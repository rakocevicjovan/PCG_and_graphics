#include "Material.h"



Material::Material()
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

	//sort by first texture only? could be faster and easier with a smaller key... and often is the same as checking them all
	for (int i = 0; i < _texDescription.size(); ++i)
	{
		context->PSSetShaderResources(texturesAdded + i, 1, &(_texDescription[i]._tex->srv));
	}
}