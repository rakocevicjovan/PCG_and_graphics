#pragma once
#include <vector>
#include <d3d11.h>
#include "SkeletalMesh.h"
#include "AnimationInstance.h"
#include "Skeleton.h"



class SkeletalModel
{
private:

	bool processNode(aiNode* node, SMatrix globNodeTransform);

public:

	std::vector<SkeletalMesh> _meshes;

	std::string _path;

	SMatrix _transform;
	
	std::vector<Animation> _anims;

	Skeleton* _skeleton;


	bool loadModel(ID3D11Device* dvc, const std::string& path);
	
	bool loadFromAiScene(ID3D11Device* dvc, const aiScene* scene, const std::string& path);

	template <typename Archive>
	void serialize(
		Archive& ar, 
		std::vector<UINT> meshIndices, 
		std::vector<UINT> animIndices, 
		UINT skelIndex)
	{
		ar(_transform, _meshes.size(), meshIndices, animIndices);	//_meshes
	}
};