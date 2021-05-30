#pragma once
#include "AssetID.h"
#include "VertSignature.h"
#include "Math.h"
#include <vector>


struct MeshAsset
{
	VertSignature vertSig;
	std::vector<uint8_t> vertices;
	std::vector<uint32_t> indices;

	SMatrix parentSpaceTransform;

	AssetID material;

	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(vertSig, vertices, indices, parentSpaceTransform, material);
	}
};


struct ModelAsset
{
	std::vector<MeshAsset> _meshes;


	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(vertSig, vertices, indices);
	}
};