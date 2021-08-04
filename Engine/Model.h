#pragma once
#include "Mesh.h"
#include "MeshNode.h"

namespace Procedural { class Terrain; }

class Model
{
public:

	std::vector<Mesh> _meshes;
	std::vector<MeshNode> _meshNodeTree;

	SMatrix _transform;

	Model() = default;
	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	Model(Model&&) = default;
	Model& operator=(Model&&) = default;
};