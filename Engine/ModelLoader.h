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
		model->_meshes.resize(modelAsset.meshes.size());

		for (auto i = 0; i < modelAsset.meshes.size(); ++i)
		{
			auto& mesh = model->_meshes[i];
			auto& meshAsset = modelAsset.meshes[i];

			mesh._vertices = meshAsset.vertices;
			mesh._indices = meshAsset.indices;

			mesh._material = MaterialLoader::LoadMaterialFromID(meshAsset.material, assetLedger);
			mesh._vertSig = meshAsset.vertSig;
		}

		model->_meshNodeTree = modelAsset.meshNodes;

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
		skModel->_meshes.resize(skModelAsset.model.meshes.size());

		for (auto i = 0; i < skModelAsset.model.meshes.size(); ++i)
		{
			auto& mesh = skModel->_meshes[i];
			auto& meshAsset = skModelAsset.model.meshes[i];

			mesh._vertices = meshAsset.vertices;
			mesh._indices = meshAsset.indices;

			mesh._material = MaterialLoader::LoadMaterialFromID(meshAsset.material, assetLedger);
			mesh._vertSig = meshAsset.vertSig;
		}

		// This will not work like this later, must check the cache first for skeleton and animations. Same for materials!
		// It should be optional to check the cache (maybe pass cache as pointer not sure)
		skModel->_skeleton = std::make_shared<Skeleton>();
		auto skeletonID = skModelAsset.skeleton;
		auto skeletonPath = assetLedger.get(skeletonID);
		std::ifstream ifs(*skeletonPath, std::ios::binary);
		cereal::BinaryInputArchive bia(ifs);
		skModel->_skeleton->serialize(bia);

		skModel->_anims.reserve(skModelAsset.animations.size());
		for (auto animID : skModelAsset.animations)
		{
			auto animPath = assetLedger.get(animID);
			std::ifstream animIfs(*animPath, std::ios::binary);
			cereal::BinaryInputArchive animBIA(animIfs);
			skModel->_anims.push_back(std::make_shared<Animation>());
			skModel->_anims.back()->serialize(animBIA);
		}

		skModel->_meshNodeTree = skModelAsset.model.meshNodes;

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