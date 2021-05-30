#pragma once
#include "VBuffer.h"
#include "IBuffer.h"
#include "Math.h"
#include "Material.h"
#include "VertSignature.h"

class Skeleton;

namespace cereal
{
	template<class Archive>
	void serialize(Archive& archive, VertBoneData& bd)
	{
		archive(bd.ids, bd.weights);
	}

	template<class Archive>
	void serialize(Archive& archive, BonedVert3D& v)
	{
		archive(v.pos, v.texCoords, v.normal, v.boneData);
	}
}


class SkeletalMesh
{
public:

	// Asset mesh holds these long term
	VertSignature _vertSig;
	std::vector<uint8_t> _vertices;
	std::vector<uint32_t> _indices;

	// And refers to these
	std::shared_ptr<Material> _material;

	// GPU mesh part - buffers - likely to change to allow spans in mega buffers etc.
	VBuffer _vertexBuffer;
	IBuffer _indexBuffer;

	// Transform relative to parent model, concatenated by tree traversal
	SMatrix _parentSpaceTransform;

	// For rendering - should be moved out
	SMatrix _worldSpaceTransform;	

	SkeletalMesh() {}


	void setupSkeletalMesh(ID3D11Device* dvc);

	void draw(ID3D11DeviceContext* dc);

	inline Material* getMaterial() { return _material.get(); }

	inline const SMatrix& renderTransform() const
	{
		return _worldSpaceTransform;
	}

	template <typename Archive>
	void serialize(Archive& archive, UINT matID)
	{
		archive(_indices, _vertices, _parentSpaceTransform, matID);
	}
};