#pragma once
/*
#include "ShaderManager.h"
#include "TextureManager.h"
#include "Texture.h"
#include "Material.h"

#include <cereal/cereal.hpp>
//#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/string.hpp>

struct MaterialAsset
{
	bool _opaque{ true };
	ShaderKey _shaderKey{};	// Currently shared by all shaders in the pack, will very likely change
	D3D11_PRIMITIVE_TOPOLOGY _primitiveTopology{ D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST };

	std::vector<TextureMetaData> _texMetaData;
	std::vector<AssetID> _textureIDs;
	
	
	template <typename Archive>
	void serialize(Archive& ar) const
	{
		ar(_shaderKey, _primitiveTopology, _opaque);
		ar(_texMetaData, _textureIDs);
	}

	// Work this out, maybe shouldn't even exist
	//MaterialAsset(const Material& mat)
	//	: _texMetaData(mat._materialTextures), _primitiveTopology(mat._primitiveTopology), _opaque(mat._opaque)
	//{
	//	if (mat.getVS())	// Id is currently same for both vs and ps, which isnt optimal but ok
	//		mat.getVS()->_id;
	//	else
	//		_shaderKey = 0u;	// Lowest common denominator, only uses position
	//}
	

	Material createRuntimeMaterial(ShaderManager* shMan, TextureManager* texMan);
};
*/