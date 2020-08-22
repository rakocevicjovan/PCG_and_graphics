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
	
	std::vector<Animation*> _anims;

	Skeleton* _skeleton;

	bool importFromFileAssimp(ID3D11Device* dvc, const std::string& path);
	
	bool importFromAiScene(ID3D11Device* dvc, const aiScene* scene, const std::string& path);

	//template <typename Archive>
	void serialize(
		cereal::BinaryOutputArchive &ar,
		std::vector<UINT>& matIndices,
		std::vector<UINT>& animIndices, 
		UINT& skelIndex)
	{
		UINT numMeshes = _meshes.size();
		ar(numMeshes);
		for (UINT i = 0; i < numMeshes; ++i)
		{
			// MatIndices[mesh.matIndex] instead when it's fixed to be external
			ar(_meshes[i], 0);	
		}
		ar(animIndices, skelIndex);
	}
};