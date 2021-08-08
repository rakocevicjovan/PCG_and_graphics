#pragma once

#include "TCachedLoader.h"
#include "Material.h"
#include "MaterialAsset.h"
#include "MaterialLoader.h"
#include "Deserialize.h"


class MaterialManager final : public TCachedLoader<Material, MaterialManager>
{
private:
	using base = TCachedLoader<Material, MaterialManager>;

	ShaderManager* _shaderManager;
	TextureManager* _textureManager;

public:
	using base::base;

	MaterialManager(AssetLedger& ledger, AeonLoader& aeonLoader, ShaderManager& shaderManager, TextureManager& textureManager)
		: base::TCachedLoader(ledger, aeonLoader), _shaderManager(&shaderManager), _textureManager(&textureManager)
	{}

	Material loadImpl(const char* path)
	{
		MaterialAsset materialAsset = AssetHelpers::DeserializeFromFile<MaterialAsset, cereal::JSONInputArchive>(path);
		return MaterialLoader::LoadMaterialFromAsset(std::move(materialAsset), _shaderManager, _textureManager);
	}
};