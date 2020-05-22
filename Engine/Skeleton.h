#pragma once
#include <string>
#include "MeshDataStructs.h"
#include "Math.h"


// @TODO get rid of the map, it was a learning crutch, too slow - transform to vector of bones with indices as "names"
class Skeleton
{
private:

public:

	std::map<std::string, Bone> _boneMap;
	Bone _root;
	SMatrix _globalInverseTransform;


	void makeLikeATree(const aiNode* node)
	{
		auto it = _boneMap.find(std::string(node->mName.data));

		// If node is a bone
		if (it != _boneMap.end())
		{
			Bone& currentBone = it->second;
			currentBone.locNodeTransform = (SMatrix(&node->mTransformation.a1).Transpose());

			if (node->mParent != nullptr)
			{
				auto it2 = _boneMap.find(std::string(node->mParent->mName.data));

				//currentJoint.locNodeTransform *= (SMatrix(&node->mParent->mTransformation.a1).Transpose());

				if (it2 != _boneMap.end())
				{
					currentBone.parent = &(it2->second);
					currentBone.parent->offspring.push_back(&currentBone);
				}
			}
		}

		for (unsigned int i = 0; i < node->mNumChildren; ++i)
			this->makeLikeATree(node->mChildren[i]);
	}



	void propagateTransformations()
	{
		for (auto nameBone : _boneMap)
		{
			if (nameBone.second.parent == nullptr)
				_root = nameBone.second;
		}

		calcGlobalTransforms(_root, SMatrix::Identity);	// Identity because this is for root only
	}



	void calcGlobalTransforms(Bone& bone, const SMatrix& parentTransform)
	{
		bone.globalTransform = bone.locNodeTransform * parentTransform;

		for (Bone* childBone : bone.offspring)
		{
			calcGlobalTransforms(*childBone, bone.globalTransform);
		}
	}


	// Returns -1 if not found
	inline int getBoneIndex(const std::string& name)
	{
		auto found = _boneMap.find(name);
		return found != _boneMap.end() ? found->second.index : -1;
	}
};