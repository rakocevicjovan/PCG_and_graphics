#pragma once

#include "Material.h"
#include "MaterialAsset.h"
#include "AssetLedger.h"
#include "TextureLoader.h"
#include "Deserialize.h"
#include <variant>

namespace MaterialLoader
{
	inline Material LoadMaterialFromAsset(const MaterialAsset& materialAsset, ShaderManager* shaderManager, TextureManager* textureManager)
	{
		Material material;

		std::vector<std::shared_future<std::shared_ptr<Texture>>> futureTextures(materialAsset._textures.size());
		material._materialTextures.resize(materialAsset._textures.size());

		for (auto i = 0u; i < materialAsset._textures.size(); ++i)
		{
			auto& texRef = materialAsset._textures[i];
			
			material._materialTextures[i]._metaData = texRef._texMetaData;

			futureTextures[i] = textureManager->getAsync(texRef._textureAssetID);
		}

		material._opaque = materialAsset._opaque;
		
		// @TODO Review how this works, it's clunky
		auto vsID = materialAsset._shaderIDs[0];
		auto psID = materialAsset._shaderIDs[1];
		
		std::shared_ptr<std::variant<VertexShader, PixelShader>> vs = shaderManager->getBlocking(vsID, ShaderType::VS);
		std::shared_ptr<std::variant<VertexShader, PixelShader>> ps = shaderManager->getBlocking(psID, ShaderType::PS);

		auto& vsRef = std::get<VertexShader>(*vs);
		auto& psRef = std::get<PixelShader>(*ps);

		material.setVS(std::shared_ptr<VertexShader>(&vsRef));
		material.setPS(std::shared_ptr<PixelShader>(&psRef));

		// Wait for textures to finish
		for (auto i = 0u; i < futureTextures.size(); ++i)
		{
			futureTextures[i].wait();
			material._materialTextures[i]._tex = futureTextures[i].get();
		}

		return material;
	}
}