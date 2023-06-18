#pragma once

#include "TCachedLoader.h"
#include "Deserialize.h"

#include "SkeletalModel.h"
#include "ModelLoader.h"

#include "MaterialManager.h"
#include "SkeletonManager.h"
#include "AnimationManager.h"


class SkModelManager final : public TCachedLoader<SkModel, SkModelManager>
{
private:

	MaterialManager* _materialManager{};
	SkeletonManager* _skMan{};
	AnimationManager* _aniMan{};


public:

	using base = TCachedLoader<SkModel, SkModelManager>;
	using TCachedLoader<SkModel, SkModelManager>::TCachedLoader;


	SkModelManager(AssetLedger& ledger, AeonLoader& aeonLoader, MaterialManager& materialManager, SkeletonManager& skeletonManager, AnimationManager& animationManager)
		: base::TCachedLoader(ledger, aeonLoader), _materialManager(&materialManager), _skMan(&skeletonManager), _aniMan(&animationManager)
	{}


	SkModel loadImpl(const char* path)
	{
		auto skModelAsset = AssetHelpers::DeserializeFromFile<SkModelAsset>(path);
		return ModelLoader::LoadSkModelFromAsset(skModelAsset, _materialManager, _skMan, _aniMan);
	}
};