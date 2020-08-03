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
	Texture* _tex;
	TextureRole _role;
	TextureMapMode _mapMode[3] = { TextureMapMode::WRAP, TextureMapMode::WRAP, TextureMapMode::WRAP };
	uint8_t _uvIndex = 0u;
	uint8_t _regIndex = 0u;	// Decouple from role later
};



class Material
{
protected:

	// Most important sorting criteria
	VertexShader* _vertexShader;
	PixelShader* _pixelShader;

public:

	// Second most important sorting criteria
	std::vector<TextureMetaData> _texMetaData;
	
	// Determines whether it goes to the transparent or opaque queue
	bool _opaque;

	// This could also belong in the vertex buffer... like stride and offset do
	D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


	Material();

	Material(VertexShader* vs, PixelShader* ps, bool opaque);


	void bindTextures(ID3D11DeviceContext* context) const;
	
	std::vector<D3D11_SAMPLER_DESC> createSamplerDescs() const;

	inline VertexShader* getVS() const { return _vertexShader; }

	inline PixelShader* getPS() const { return _pixelShader; }

	inline void setVS(VertexShader* vs) { _vertexShader = vs; }

	inline void setPS(PixelShader* ps) { _pixelShader = ps; }

	inline void bindSamplers(ID3D11DeviceContext* context)
	{
		for (UINT i = 0; i < _pixelShader->_samplers.size(); ++i)
			context->PSSetSamplers(i, 1, &_pixelShader->_samplers[i]);
	}

	template <typename Archive>
	void serialize(Archive& ar, std::vector<UINT>& texIDs)
	{
		ar(0u, 0u, texIDs);
	}
};