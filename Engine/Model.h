#pragma once

#include <string>
#include <vector>
#include <d3d11_4.h>

#include "Mesh.h"
#include "Collider.h"

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>


namespace Procedural { class Terrain; }


class Model : public Resource
{
private:

	bool processNode(aiNode* node, SMatrix parentTransform);

public:

	std::string _path;

	std::vector<Mesh> _meshes;

	SMatrix _transform;

	//remove this eventually when game object becomes better defined... used model for it so far...
	Collider* collider;

	Model() : collider(nullptr) {}
	Model(const std::string& path);
	~Model();

	// Separate model and terrain completely, terrain needs a different way to render
	Model(const Procedural::Terrain& terrain, ID3D11Device* device);	

	bool loadFromAssimp(ID3D11Device* device, const std::string& path);
	bool loadFromAiScene(ID3D11Device* device, const aiScene* scene, const std::string& path);

	template<class Archive>
	void serialize(Archive& archive, std::vector<AssetID>& matIndices)
	{
		archive(_transform);
	}
};