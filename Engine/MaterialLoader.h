#pragma once

#include "Material.h"
#include "MaterialAsset.h"
#include "AssetLedger.h"
#include "TextureLoader.h"
#include "Deserialize.h"


namespace MaterialLoader
{
	static Material LoadMaterialFromAsset(const MaterialAsset& materialAsset, ShaderManager* shaderManager, TextureManager* textureManager)
	{
		//auto material = std::make_unique<Material>();
		Material material;

		std::vector<std::future<std::shared_ptr<Texture>>> futureTextures(materialAsset._textures.size());
		material._materialTextures.resize(materialAsset._textures.size());

		for (auto i = 0u; i < materialAsset._textures.size(); ++i)	//auto& texRef : materialAsset._textures
		{
			auto& texRef = materialAsset._textures[i];
			MaterialTexture materialTexture;
			materialTexture._metaData = texRef._texMetaData;
			futureTextures[i] = textureManager->get_async(texRef._textureAssetID);
			//material._materialTextures.emplace_back(std::move(materialTexture));
		}

		for (auto i = 0u; i < futureTextures.size(); ++i)
		{
			futureTextures[i].wait();
			material._materialTextures[i]._tex = futureTextures[i].get();
		}

		material._opaque = materialAsset._opaque;
		
		// Load shaders too

		return material;
	}
}