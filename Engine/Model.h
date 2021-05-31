#pragma once
#include "Mesh.h"

#include <vector>
#include <d3d11_4.h>

namespace Procedural { class Terrain; }

class Model : public Resource
{
public:

	std::vector<Mesh> _meshes;

	SMatrix _transform;

	Model() = default;
	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	Model(Model&&) = default;
	Model& operator=(Model&&) = default;

	// A convenience function should possibly exist in terrain to make a mesh, but certainly not here.
	Model(const Procedural::Terrain& terrain, ID3D11Device* device);	

	template<class Archive>
	void serialize(Archive& archive, std::vector<AssetID>& matIndices)
	{
		//archive(_transform);
	}
};