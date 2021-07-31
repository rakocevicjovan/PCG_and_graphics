#pragma once

#include "Material.h"
#include "MaterialAsset.h"
#include "AssetLedger.h"
#include "TextureLoader.h"
#include "Deserialize.h"


namespace MaterialLoader
{
	static std::unique_ptr<Material> LoadMaterialFromAsset(const MaterialAsset& materialAsset, ShaderManager* shaderManager, TextureManager* textureManager)
	{
		auto material = std::make_unique<Material>();

		material->_materialTextures.reserve(materialAsset._textures.size());

		for (auto& texRef : materialAsset._textures)
		{
			MaterialTexture materialTexture;
			materialTexture._metaData = texRef._texMetaData;
			materialTexture._tex = textureManager->get(texRef._textureAssetID);

			material->_materialTextures.emplace_back(std::move(materialTexture));
		}

		material->_opaque = materialAsset._opaque;

		return material;
	}
}