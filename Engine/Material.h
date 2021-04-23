#pragma once
#include "MeshDataStructs.h"
#include "TextureMetaData.h"
#include "TextureManager.h"
#include "ShaderManager.h"

#include <memory>


struct MaterialTexture
{
	TextureMetaData _metaData;
	std::shared_ptr<Texture> _tex;

	// Do not serialize textures. They will be obtained otherwise through the managers to prevent double loading.
	template<typename Archive>
	void serialize(Archive& ar)
	{
		ar(_metaData);
	}
};

class Material
{
protected:

	// Most important sorting criteria
	VertexShader* _vertexShader{};
	PixelShader* _pixelShader{};

public:

	// Second most important sorting criteria
	std::vector<MaterialTexture> _materialTextures;
	
	// Determines whether it goes to the transparent or opaque queue
	bool _opaque{false};

	// This could also belong in the vertex buffer... like stride and offset do
	D3D11_PRIMITIVE_TOPOLOGY _primitiveTopology{ D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST };


	Material();

	Material(VertexShader* vs, PixelShader* ps, bool opaque);

	void bind(ID3D11DeviceContext* context)
	{
		_vertexShader->bind(context);
		_pixelShader->bind(context);
		bindTextures(context);
		context->IASetPrimitiveTopology(_primitiveTopology);
	}

	void bindTextures(ID3D11DeviceContext* context) const;
	
	std::vector<D3D11_SAMPLER_DESC> createSamplerDescs() const;

	inline VertexShader* getVS() const { return _vertexShader; }

	inline PixelShader* getPS() const { return _pixelShader; }

	inline void setVS(VertexShader* vs) { _vertexShader = vs; }

	inline void setPS(PixelShader* ps) { _pixelShader = ps; }

	inline void addMaterialTexture(Texture* t, const TextureMetaData& tmd)
	{
		_materialTextures.push_back({ tmd, std::shared_ptr<Texture>(t) });
	}

	template <typename Archive>
	void save(Archive& ar, const std::vector<uint32_t>& texIDs) const
	{
		ShaderKey vsID = _vertexShader ? _vertexShader->_id : 0u;
		ShaderKey psID = _pixelShader ? _pixelShader->_id : 0u;
		
		ar(vsID, psID, _primitiveTopology, _opaque, _materialTextures, texIDs);
	}

	template <typename Archive>
	void load(Archive& ar, ShaderManager& shMan, TextureManager& texMan) const
	{
		ShaderKey vsID;
		ShaderKey psID;

		std::vector<AssetID> texIDs;

		ar(vsID, psID, _primitiveTopology, _opaque, _texMetaData, texIDs);
		
		for (auto i = 0; i < _texMetaData.size(); ++i)
		{
			_texMetaData[i]._tex = texMan.get(texIDs[i]);
		}

		if (auto shaderPack = shMan.getShaderByKey(vsID))
		{
			_vertexShader = shaderPack->vs;
			_pixelShader = shaderPack->ps;
		}
	}
};