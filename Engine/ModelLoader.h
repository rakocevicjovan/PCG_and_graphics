#pragma once
#include "Model.h"
#include "SkeletalModel.h"
#include "ModelAsset.h"
#include "MaterialLoader.h"
#include "Animation.h"
#include "AssetLedger.h"
#include "Deserialize.h"


namespace ModelLoader
{
	static Mesh LoadMesh(MeshAsset& meshAsset, const AssetLedger& assetLedger);

	namespace
	{
		static void LoadMeshes(std::vector<Mesh>& meshes, ModelAsset& modelAsset, const AssetLedger& assetLedger)
		{
			meshes.resize(modelAsset.meshes.size());

			for (auto i = 0; i < meshes.size(); ++i)
			{
				meshes[i] = LoadMesh(modelAsset.meshes[i], assetLedger);
			}
		}
	}


	static Mesh LoadMesh(MeshAsset& meshAsset, const AssetLedger& assetLedger)
	{
		Mesh mesh;

		mesh._vertices = std::move(meshAsset.vertices);
		mesh._indices = std::move(meshAsset.indices);
		mesh._vertSig = std::move(meshAsset.vertSig);

		mesh._material = MaterialLoader::LoadMaterialFromID(meshAsset.material, assetLedger);

		return mesh;
	}


	static std::unique_ptr<Model> LoadModelFromAsset(ModelAsset& modelAsset, const AssetLedger& assetLedger)
	{
		auto model = std::make_unique<Model>();
		
		LoadMeshes(model->_meshes, modelAsset, assetLedger);

		model->_meshNodeTree = modelAsset.meshNodes;

		return model;
	}


	static SkModel LoadSkModelFromAsset(SkModelAsset& skModelAsset, const AssetLedger& assetLedger)
	{
		SkModel skModel;
		
		LoadMeshes(skModel._meshes, skModelAsset.model, assetLedger);

		skModel._meshNodeTree = skModelAsset.model.meshNodes;

		// This will not work like this later, must check the cache first for skeleton and animations. Same for materials!
		// It should be optional to check the cache (maybe pass cache as pointer not sure)

		auto skeletonPath = assetLedger.getPath(skModelAsset.skeleton);
		skModel._skeleton = std::make_shared<Skeleton>(AssetHelpers::DeserializeFromFile<Skeleton>(skeletonPath->c_str()));

		skModel._anims.reserve(skModelAsset.animations.size());
		for (auto animID : skModelAsset.animations)
		{
			auto animPath = assetLedger.getPath(animID);
			skModel._anims.push_back(std::make_shared<Animation>(AssetHelpers::DeserializeFromFile<Animation>(animPath->c_str())));
		}

		return skModel;
	}
}