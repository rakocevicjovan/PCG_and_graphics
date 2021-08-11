#pragma once
#include "CTransform.h"
#include "CSkModel.h"


// Try using cereal without this
//struct LevelAsset {};


// Terrain
// Skybox
// Mesh nodes (sk model, static models, volumetric bounding boxes where not screen space... doesn't matter, all that)
// Volumes (logical, for trigger volumes etc.)
// Vegetation and other props that might need special rendering etc.


template<typename Archive>
void serialize(Archive& ar, CTransform& transform)
{
	ar(transform.transform);
}


template<typename Archive>
void serialize(Archive& ar, CSkModel& cskmodel)
{
	ar(rand() % 999);
}


namespace LevelAsset
{

	template <typename Archive>
	void serializeScene(Archive& ar, entt::registry& registry)
	{
		entt::snapshot{ registry }.entities(ar).component<CTransform, CSkModel>(ar);	//, entt::tag<"empty"_hs>
	}

}