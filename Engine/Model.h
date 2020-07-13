#pragma once

#include <string>
#include <vector>
#include <d3d11.h>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>

#include "AssimpWrapper.h"


namespace Procedural { class Terrain; }


class Model : public Resource
{
private:

	bool processNode(ID3D11Device* device, aiNode* node, const aiScene* scene, aiMatrix4x4 parentTransform, float rUVx, float rUVy);

	bool processMesh(ID3D11Device* device, aiMesh* aiMesh, Mesh& mesh, const aiScene* scene, aiMatrix4x4 parentTransform, float rUVx, float rUVy);

public:

	std::string _path;

	std::vector<Mesh> _meshes;

	SMatrix _transform;

	//remove this eventually when game object becomes better defined... used model for it so far...
	Collider* collider;

	Model() : collider(nullptr) {}
	Model(const std::string& path);
	Model(const Collider& collider, ID3D11Device* device);
	~Model();

	// This should be inversed, model should not know about terrain
	Model(const Procedural::Terrain& terrain, ID3D11Device* device);	

	bool LoadModel(ID3D11Device* device, const std::string& path, float rUVx = 1, float rUVy = 1);
	bool LoadFromScene(ID3D11Device* device, const aiScene* scene, float rUVx = 1, float rUVy = 1);

	template<class Archive>
	void serialize(Archive& archive, std::vector<UINT>& meshIndices)
	{
		archive(_transform, _meshes.size(), meshIndices);
	}
};