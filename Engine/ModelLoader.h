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
	static Mesh LoadMesh(MeshAsset&, MaterialManager*, std::vector<std::shared_future<std::shared_ptr<Material>>>&);

	namespace
	{
		static void LoadMeshes(std::vector<Mesh>& meshes, ModelAsset& modelAsset, MaterialManager* materialManager)
		{
			auto numMeshes = modelAsset.meshes.size();

			std::vector<std::shared_future<std::shared_ptr<Material>>> futureMats;
			futureMats.reserve(numMeshes);
			meshes.reserve(numMeshes);

			for (auto i = 0; i < numMeshes; ++i)
			{
				meshes.emplace_back(LoadMesh(modelAsset.meshes[i], materialManager, futureMats));
			}

			for (auto i = 0; i < numMeshes; ++i)
			{
				meshes[i]._material = futureMats[i].get();
			}
		}
	}


	static Mesh LoadMesh(MeshAsset& meshAsset, MaterialManager* materialManager, std::vector<std::shared_future<std::shared_ptr<Material>>>& futureMats)
	{
		Mesh mesh;

		mesh._vertices = std::move(meshAsset.vertices);
		mesh._indices = std::move(meshAsset.indices);
		mesh._vertSig = std::move(meshAsset.vertSig);

		mesh.average_position = meshAsset.average_position;
		mesh.max_distance = meshAsset.max_distance;

		futureMats.push_back(materialManager->getAsync(meshAsset.material));

		return mesh;
	}


	static Model LoadModelFromAsset(ModelAsset& modelAsset, MaterialManager* materialManager)
	{
		Model model{};
		
		LoadMeshes(model._meshes, modelAsset, materialManager);

		model._meshNodeTree = modelAsset.meshNodes;

		return model;
	}

	/*, SkeletonManager* skeletonManager, AnimationManager* animationManager */
	static SkModel LoadSkModelFromAsset(SkModelAsset& skModelAsset, MaterialManager* materialManager, SkeletonManager* skMan, AnimationManager* aniMan)
	{
		SkModel skModel{};
		
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