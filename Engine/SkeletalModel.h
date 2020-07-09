#pragma once
#include <vector>
#include <d3d11.h>

#include "AssimpWrapper.h"
#include "SkeletalMesh.h"
#include "AnimationInstance.h"
#include "Skeleton.h"
#include "SerializableAsset.h"



class SkeletalModel //: public SerializableAsset
{
private:

	bool processNode(ID3D11Device* dvc, aiNode* node, const aiScene* scene, float rUVx, float rUVy, SMatrix globNodeTransform);
	SkeletalMesh processSkeletalMesh(ID3D11Device* device, aiMesh* mesh, const aiScene* scene, unsigned int ind, SMatrix transform, float rUVx, float rUVy);

public:

	std::vector<SkeletalMesh> _meshes;

	std::string _path;

	SMatrix _transform;
	
	std::vector<Animation> _anims;

	Skeleton _skeleton;

	SkeletalModel();
	~SkeletalModel();


	bool loadModel(ID3D11Device* dvc, const std::string& path, float rUVx = 1., float rUVy = 1.);
	bool loadFromScene(ID3D11Device* dvc, const aiScene* scene, float rUVx = 1., float rUVy = 1.);

	template <typename Archive>
	void serialize(
		Archive& ar, 
		std::vector<UINT> meshIndices, 
		std::vector<UINT> animIndices, 
		UINT skelIndex)
	{
		archive(_transform, _meshes.size(), meshIndices, animIndices);
	}
};