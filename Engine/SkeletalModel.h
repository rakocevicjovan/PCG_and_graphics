#pragma once
#include "SkeletalMesh.h"
#include "AnimationInstance.h"
#include "Skeleton.h"

#include <vector>
#include <d3d11_4.h>
#include <memory>



class SkeletalModel
{
private:

	bool processNode(aiNode* node, SMatrix globNodeTransform);

public:

	std::vector<SkeletalMesh> _meshes;

	std::string _path;
	
	std::vector<Animation*> _anims;

	std::shared_ptr<Skeleton> _skeleton;

	bool importFromFileAssimp(ID3D11Device* dvc, const std::string& path);
	
	bool importFromAiScene(ID3D11Device* dvc, const aiScene* scene, const std::string& path, 
		std::vector<std::shared_ptr<Material>>& mats, std::shared_ptr<Skeleton>& skeleton);

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