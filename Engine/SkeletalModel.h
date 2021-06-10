#pragma once
#include "Mesh.h"
#include "MeshNode.h"
#include "AnimationInstance.h"
#include "Skeleton.h"


class SkModel
{
public:

	std::vector<Mesh> _meshes;
	std::vector<MeshNode> _meshNodeTree;

	std::vector<Animation*> _anims;

	std::shared_ptr<Skeleton> _skeleton;
};