#pragma once
#include "Model.h"
#include "SkeletalModel.h"
#include "ModelAsset.h"
#include "AssetLedger.h"
#include "MaterialLoader.h"

 
namespace ModelLoader
{
	static std::shared_ptr<Model> LoadModelFromAsset(const ModelAsset& modelAsset, AssetLedger& assetLedger)
	{
		auto model = std::make_shared<Model>();

		for (auto i = 0; i < modelAsset.meshes.size(); ++i)
		{
			model->_meshes[i]._vertices = modelAsset.meshes[i].vertices;
			model->_meshes[i]._indices = modelAsset.meshes[i].indices;

			//model->_meshes[i].setupMesh(_device); Problematic.

			auto matID = modelAsset.meshes[i].material;

			model->_meshes[i]._material = MaterialLoader::LoadMaterialFromID(matID, assetLedger);
		}

		return model;
	}


	static std::shared_ptr<Model> LoadModelFromID(AssetID modelID, AssetLedger& assetLedger)
	{
		auto modelPath = assetLedger.get(modelID);
		std::ifstream ifs(*modelPath, std::ios::binary);
		cereal::BinaryInputArchive bia(ifs);

		ModelAsset modelAsset;
		modelAsset.serialize(bia);

		return LoadModelFromAsset(modelAsset, assetLedger);
	}


	static std::shared_ptr<SkModel> LoadSkModelFromAsset(const SkModelAsset& skModelAsset, AssetLedger& assetLedger)
	{
		auto skModel = std::make_shared<SkModel>();
		return skModel;
	}


	static std::shared_ptr<SkModel> LoadSkModelFromID(AssetID modelID, AssetLedger& assetLedger)
	{
		auto modelPath = assetLedger.get(modelID);
		std::ifstream ifs(*modelPath, std::ios::binary);
		cereal::BinaryInputArchive bia(ifs);

		SkModelAsset skModelAsset;
		skModelAsset.serialize(bia);

		return LoadSkModelFromAsset(skModelAsset, assetLedger);
	}
}