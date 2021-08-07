#pragma once
#include "Model.h"
#include "SkeletalModel.h"
#include "ModelAsset.h"

#include "MaterialManager.h"
#include "SkeletonManager.h"
#include "AnimationManager.h"

#include "AssetLedger.h"
#include "Deserialize.h"


namespace ModelLoader
{
	static Mesh LoadMesh(MeshAsset& meshAsset, MaterialManager* materialManager);

	namespace
	{
		static void LoadMeshes(std::vector<Mesh>& meshes, ModelAsset& modelAsset, MaterialManager* materialManager)
		{
			meshes.resize(modelAsset.meshes.size());

			for (auto i = 0; i < meshes.size(); ++i)
			{
				meshes[i] = LoadMesh(modelAsset.meshes[i], materialManager);
			}
		}
	}


	static Mesh LoadMesh(MeshAsset& meshAsset, MaterialManager* materialManager)
	{
		Mesh mesh;

		mesh._vertices = std::move(meshAsset.vertices);
		mesh._indices = std::move(meshAsset.indices);
		mesh._vertSig = std::move(meshAsset.vertSig);

		mesh._material = materialManager->get(meshAsset.material);

		return mesh;
	}


	static std::unique_ptr<Model> LoadModelFromAsset(ModelAsset& modelAsset, MaterialManager* materialManager)
	{
		auto model = std::make_unique<Model>();
		
		LoadMeshes(model->_meshes, modelAsset, materialManager);

		model->_meshNodeTree = modelAsset.meshNodes;

		return model;
	}

	/*, SkeletonManager* skeletonManager, AnimationManager* animationManager */
	static SkModel LoadSkModelFromAsset(SkModelAsset& skModelAsset, MaterialManager* materialManager, SkeletonManager* skMan, AnimationManager* aniMan)
	{
		SkModel skModel;
		
		LoadMeshes(skModel._meshes, skModelAsset.model, materialManager);

		skModel._meshNodeTree = skModelAsset.model.meshNodes;

		// Schedule skeleton load
		auto skeletonFuture = skMan->getAsync(skModelAsset.skeleton);
		//skModel._skeleton = std::make_shared<Skeleton>(AssetHelpers::DeserializeFromFile<Skeleton>(skeletonPath->c_str()));

		// Schedule animation loads
		std::vector<std::shared_future<std::shared_ptr<Animation>>> animFutures;
		animFutures.reserve(skModelAsset.animations.size());

		for (auto animID : skModelAsset.animations)
		{
			animFutures.push_back(std::move(aniMan->getAsync(animID)));
			//skModel._anims.push_back(std::make_shared<Animation>(AssetHelpers::DeserializeFromFile<Animation>(animPath->c_str())));
		}

		// Wait for skeleton
		skModel._skeleton = skeletonFuture.get();

		// Wait for animations
		skModel._anims.reserve(skModelAsset.animations.size());

		for (auto& animFuture : animFutures)
		{
			skModel._anims.push_back(animFuture.get());
		}

		return skModel;
	}
}