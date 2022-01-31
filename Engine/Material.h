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

	std::shared_ptr<VertexShader> _vertexShader{};
	std::shared_ptr<PixelShader> _pixelShader{};

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

	inline VertexShader* getVS() const { return _vertexShader.get(); }

	inline PixelShader* getPS() const { return _pixelShader.get(); }

	inline void setVS(std::shared_ptr<VertexShader> vs) { _vertexShader = std::move(vs); }

	inline void setPS(std::shared_ptr<PixelShader> ps) { _pixelShader = std::move(ps); }

	inline void addMaterialTexture(Texture* t, const TextureMetaData& tmd)
	{
		_materialTextures.push_back({ tmd, std::shared_ptr<Texture>(t) });
	}
};