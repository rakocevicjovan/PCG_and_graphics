#pragma once
#include "Math.h"
#include <vector>


struct MeshNode
{
	uint16_t parent;
	SMatrix transform;	// Parent space transform (model is the parent)

	//This might be a good opportunity to optimize with SVO 
	std::vector<uint16_t> meshes;

	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(parent, transform, meshes);
	}
};
