#pragma once
#include "Material.h"
#include "MaterialAsset.h"
#include "AssetLedger.h"
#include "TextureLoader.h"


namespace MaterialLoader
{
	static std::shared_ptr<Material> LoadMaterialFromAsset(const MaterialAsset& materialAsset, AssetLedger& assetLedger)
	{
		std::shared_ptr<Material> material = std::make_shared<Material>();

		material->_materialTextures.reserve(materialAsset._textures.size());

		for (auto& texRef : materialAsset._textures)
		{
			MaterialTexture materialTexture;
			materialTexture._metaData = texRef._texMetaData;
			materialTexture._tex = LoadTextureFromAsset(texRef._textureAssetID, assetLedger);

			material->_materialTextures.emplace_back(std::move(materialTexture));
		}

		material->_opaque = materialAsset._opaque;

		return material;
	}


	static std::shared_ptr<Material> LoadMaterialFromID(AssetID materialID, AssetLedger& assetLedger)
	{
		auto path = assetLedger.get(materialID);

		std::ifstream ifs(*path, std::ios::binary);
		cereal::JSONInputArchive jia(ifs);

		MaterialAsset materialAsset;
		materialAsset.serialize(jia);

		return LoadMaterialFromAsset(materialAsset, assetLedger);
	}
}