#include "Material.h"



Material::Material()
{
}


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