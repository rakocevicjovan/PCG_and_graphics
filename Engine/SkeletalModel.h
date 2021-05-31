#pragma once
#include "Mesh.h"
#include "AnimationInstance.h"
#include "Skeleton.h"


class SkeletalModel
{
public:

	std::vector<Mesh> _meshes;
	
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