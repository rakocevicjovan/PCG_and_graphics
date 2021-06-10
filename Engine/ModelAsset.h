#pragma once
#include "AssetID.h"
#include "VertSignature.h"
#include "Math.h"
#include "Model.h"
#include <vector>


struct MeshAsset
{
	VertSignature vertSig;
	std::vector<uint8_t> vertices;
	std::vector<uint32_t> indices;

	AssetID material;

	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(vertSig, vertices, indices, material);
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

	AssetID skeleton;
	std::vector<AssetID> animations;

	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(model, skeleton, animations);
	}
};