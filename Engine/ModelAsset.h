#pragma once
#include "AssetID.h"
#include "VertSignature.h"
#include "Math.h"
#include "Model.h"

struct MeshAsset
{
	VertSignature vertSig;
	std::vector<uint8_t> vertices;
	std::vector<uint32_t> indices;

	AssetID material{ NULL_ASSET };

	SVec3 average_position;
	float max_distance;

	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(vertSig, vertices, indices, material, average_position, max_distance);
	}
};

struct ModelAsset
{
	std::vector<MeshAsset> meshes;
	std::vector<MeshNode> meshNodes;
	
	SMatrix transform;

	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(meshes, meshNodes, transform);
	}
};

struct SkModelAsset
{
	ModelAsset model;

	AssetID skeleton{ NULL_ASSET };
	std::vector<AssetID> animations;

	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(model, skeleton, animations);
	}
};
