#include "pch.h"
#include "Sampler.h"
#include "Material.h"



Material::Material(VertexShader* vs, PixelShader* ps, bool opaque) 
	: _vertexShader(vs), _pixelShader(ps), _opaque(opaque) {}



void Material::bindTextures(ID3D11DeviceContext* context) const
{
	for(auto& [metaData, tex] : _materialTextures)
	{
		if (tex->_dxID.Get())
		{
			context->PSSetShaderResources(metaData._role, 1, tex->_srv.GetAddressOf());
		}
	}
}


// What is this even doing 
std::vector<D3D11_SAMPLER_DESC> Material::createSamplerDescs() const
{
	static const std::map<SamplingMode, D3D11_TEXTURE_ADDRESS_MODE> ADDR_MODE_MAP
	{
		{SamplingMode::CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP},
		{SamplingMode::WRAP, D3D11_TEXTURE_ADDRESS_WRAP},
		{SamplingMode::MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR},
		{SamplingMode::MIRROR_ONCE, D3D11_TEXTURE_ADDRESS_MIRROR_ONCE},
		{SamplingMode::BORDER, D3D11_TEXTURE_ADDRESS_BORDER}
	};

	std::vector<D3D11_SAMPLER_DESC> result;
	result.reserve(_materialTextures.size());

	// Uses default for now, but make it a setting eventually
	D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	for (UINT i = 0; i < _materialTextures.size(); ++i)
	{
		D3D11_TEXTURE_ADDRESS_MODE tam[3];

		for (UINT j = 0; j < 3; ++j)
			tam[j] = ADDR_MODE_MAP.at(_materialTextures[i]._metaData._mapMode[j]);

		result.push_back(Sampler::CreateSamplerDesc(filter, tam[0], tam[1], tam[2]));	// goes the drum...
	}

	return result;
}