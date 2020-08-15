#pragma once
#include "VBuffer.h"
#include "IBuffer.h"
#include "MeshDataStructs.h"
#include "Math.h"
#include "Material.h"
#include "AssimpWrapper.h"
#include "VertSignature.h"

#include <vector>
#include <string>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>

/*
class aiMesh;
class aiScene;
*/
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
	VertSignature _vertSig;

	std::vector<uint8_t> _vertices;
	std::vector<UINT> _indices;
	std::vector<Texture> _textures;

	SMatrix _transform;	// relative to parent model
	SMatrix _localTransform;

	Material _baseMaterial;

	VBuffer _vertexBuffer;
	IBuffer _indexBuffer;

	SkeletalMesh() {}

	
	void loadFromAssimp(const aiScene* scene, ID3D11Device* device, aiMesh* aiMesh, Skeleton& skeleton, const std::string& path);


	bool setupSkeletalMesh(ID3D11Device* dvc);


	void draw(ID3D11DeviceContext* dc);

	

	template <typename Archive>
	void serialize(Archive& archive, UINT matID)
	{
		archive(_indices.size(), _vertices.size(), matID, _transform, _indices, _vertices);
	}
};