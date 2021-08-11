#pragma once

#include "TCachedLoader.h"
#include "Deserialize.h"

#include "Model.h"
#include "ModelLoader.h"

#include "MaterialManager.h"


class ModelManager final : public TCachedLoader<Model, ModelManager>
{
private:

	MaterialManager* _materialManager{};


public:

	using base = TCachedLoader<Model, ModelManager>;
	using TCachedLoader<Model, ModelManager>::TCachedLoader;


	ModelManager(AssetLedger& ledger, AeonLoader& aeonLoader, MaterialManager& materialManager)
		: base::TCachedLoader(ledger, aeonLoader), _materialManager(&materialManager)
	{}


	Model loadImpl(const char* path)
	{
		auto modelAsset = AssetHelpers::DeserializeFromFile<ModelAsset>(path);
		return ModelLoader::LoadModelFromAsset(std::move(modelAsset), _materialManager);
	}
};