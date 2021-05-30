#pragma once
#include "Mesh.h"

#include <vector>
#include <d3d11_4.h>

namespace Procedural { class Terrain; }

class Model : public Resource
{
private:

	bool processNode(aiNode* node, SMatrix parentTransform);

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

	// Remove from here to an actual importer
	bool loadFromAssimp(ID3D11Device* device, const std::string& path);
	bool loadFromAiScene(ID3D11Device* device, const aiScene* scene, const std::string& path);

	template<class Archive>
	void serialize(Archive& archive, std::vector<AssetID>& matIndices)
	{
		archive(_transform);
	}
};