#pragma once
#include "MeshDataStructs.h"
#include "TextureMetaData.h"
#include "TextureManager.h"
#include "ShaderManager.h"


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

	// Most important sorting criteria - @TODO convert to shared ptr or some other ref count method when it makes sense
	VertexShader* _vertexShader{};
	PixelShader* _pixelShader{};

public:

	// Second most important sorting criteria
	std::vector<MaterialTexture> _materialTextures;
	
	// Determines whether it goes to the transparent or opaque queue
	bool _opaque{false};

	Material() = default;

	Material(VertexShader* vs, PixelShader* ps, bool opaque);

	void bind(ID3D11DeviceContext* context)
	{
		_vertexShader->setBuffers(context);
		_pixelShader->setBuffers(context);

		_vertexShader->bind(context);
		_pixelShader->bind(context);

		bindTextures(context);
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
};