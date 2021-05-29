#pragma once
#include "SkeletalMesh.h"
#include "AnimationInstance.h"
#include "Skeleton.h"

#include <vector>
#include <d3d11_4.h>
#include <memory>



class SkeletalModel
{
public:

	std::vector<SkeletalMesh> _meshes;

	std::string _path;
	
	std::vector<Animation*> _anims;

	std::shared_ptr<Skeleton> _skeleton;
	
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