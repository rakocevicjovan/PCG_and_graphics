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

		std::vector<std::shared_future<std::shared_ptr<Texture>>> futureTextures(materialAsset._textures.size());
		material._materialTextures.resize(materialAsset._textures.size());

		for (auto i = 0u; i < materialAsset._textures.size(); ++i)
		{
			auto& texRef = materialAsset._textures[i];
			MaterialTexture materialTexture;
			materialTexture._metaData = texRef._texMetaData;
			futureTextures[i] = textureManager->getAsync(texRef._textureAssetID);
		}

		material._opaque = materialAsset._opaque;
		
		// Load shaders too

		// Wait for textures to finish
		for (auto i = 0u; i < futureTextures.size(); ++i)
		{
			futureTextures[i].wait();
			material._materialTextures[i]._tex = futureTextures[i].get();
		}

		return material;
	}
}