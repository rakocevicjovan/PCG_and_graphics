#pragma once
#include "Texture.h"
#include "Shader.h"
#include "Light.h"
#include "Sampler.h"
#include "MeshDataStructs.h"

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <map>

struct TextureMetaData
{
	TextureRole _role;
	Texture* _tex;
	TextureMapMode _mapMode = TextureMapMode::WRAP;
	uint8_t _uvIndex = 0u;
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
	std::vector<TextureMetaData> _texMetaData;
	
	// determines whether it goes to the transparent or opaque queue
	bool _opaque;

	//this could also belong in the vertex buffer... like stride and offset do
	D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// Functions
	Material();
	Material(VertexShader* vs, PixelShader* ps, bool opaque);
	~Material();

	void bindTextures(ID3D11DeviceContext* context);
	
	inline VertexShader* getVS() const { return _vertexShader; }
	inline PixelShader* getPS() const { return _pixelShader; }

	void setVS(VertexShader* vs);
	void setPS(PixelShader* ps);



	std::vector<D3D11_SAMPLER_DESC> createSamplerDesc() const
	{
		static const std::map<TextureMapMode, D3D11_TEXTURE_ADDRESS_MODE> ADDR_MODE_MAP
		{
			{CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP},
			{WRAP, D3D11_TEXTURE_ADDRESS_WRAP},
			{MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR},
			{MIRROR_ONCE, D3D11_TEXTURE_ADDRESS_MIRROR_ONCE},
			{BORDER, D3D11_TEXTURE_ADDRESS_BORDER}
		};

		std::vector<D3D11_SAMPLER_DESC> result;
		result.reserve(_texMetaData.size());

		// Uses default for now, but make it a setting eventually
		D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

		for (UINT i = 0; i < _texMetaData.size(); ++i)
		{
			const TextureMetaData& tmd = _texMetaData[i];
			D3D11_TEXTURE_ADDRESS_MODE tam = ADDR_MODE_MAP.at(tmd._mapMode);
			result.push_back(Sampler::createSamplerDesc(filter, D3D11_COMPARISON_ALWAYS, 0.,
				D3D11_FLOAT32_MAX, tam, tam, tam));	// goes the drum...
		}

		return result;
	}



	template <typename Archive>
	void serialize(Archive& ar, std::vector<UINT>& texIDs)
	{
		ar(0u, 0u, texIDs);
	}
};