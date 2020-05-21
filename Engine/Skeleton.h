#pragma once
#include <string>
#include "MeshDataStructs.h"
#include "Math.h"


// @TODO get rid of the map, it was a learning crutch, too slow - transform to vector of bones with indices as "names"
class Skeleton
{
private:

public:

	std::map<std::string, Joint> _boneMap;
	Joint _rootJoint;
	SMatrix _globalInverseTransform;


	void link(const aiNode* node)
	{
		auto it = _boneMap.find(std::string(node->mName.data));

		// If node is a bone
		if (it != _boneMap.end())
		{
			Joint& currentJoint = it->second;
			currentJoint.locNodeTransform = (SMatrix(&node->mTransformation.a1).Transpose());

			if (node->mParent != nullptr)
			{
				auto it2 = _boneMap.find(std::string(node->mParent->mName.data));

				//currentJoint.locNodeTransform *= (SMatrix(&node->mParent->mTransformation.a1).Transpose());

				if (it2 != _boneMap.end())
				{
					currentJoint.parent = &(it2->second);
					currentJoint.parent->offspring.push_back(&currentJoint);
				}
			}
		}

		for (unsigned int i = 0; i < node->mNumChildren; ++i)
			this->link(node->mChildren[i]);
	}



	void makeLikeATree()
	{
		for (auto nameJointPair : _boneMap)
		{
			if (nameJointPair.second.parent == nullptr)
				_rootJoint = nameJointPair.second;
		}

		calcGlobalTransforms(_rootJoint, SMatrix::Identity);	// Identity because this is for root only
	}



	void calcGlobalTransforms(Joint& j, const SMatrix& parentMat)
	{
		j.globalTransform = j.locNodeTransform * parentMat;

		for (Joint* cj : j.offspring)
		{
			calcGlobalTransforms(*cj, j.globalTransform);
		}
	}


	// Returns -1 if not found
	inline int getBoneIndex(const std::string& name)
	{
		auto found = _boneMap.find(name);
		return found != _boneMap.end() ? found->second.index : -1;
	}
};