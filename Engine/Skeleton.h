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
		std::string nodeName = std::string(node->mName.data);

		auto nameBone = _boneMap.find(nodeName);

		if (nameBone != _boneMap.end())	// If node is a bone
		{
			Bone& currentBone = nameBone->second;
			currentBone.locNodeTransform = (SMatrix(&node->mTransformation.a1).Transpose());
			linkToParentBone(node, currentBone);
		}

		for (unsigned int i = 0; i < node->mNumChildren; ++i)
			this->makeLikeATree(node->mChildren[i]);
	}


	void linkToParentBone(const aiNode* node, Bone& currentBone)
	{
		aiNode* parent = node->mParent;

		if (parent == nullptr)	// We are at root node, nowhere to go
			return;

		std::string parentName(node->mParent->mName.data);

		auto existingBone = _boneMap.find(parentName);

		if (existingBone != _boneMap.end())	// Found it, no need to go further
		{
			currentBone.parent = &(existingBone->second);
			currentBone.parent->offspring.push_back(&currentBone);
		}
		else	// This node is not a bone yet, but it should be, so add it.
		{
			Bone newParentBone(_boneMap.size(), parentName, SMatrix(&parent->mTransformation.a1).Transpose());
			_boneMap.insert({ parentName, newParentBone });
			currentBone.parent = &(_boneMap.at(parentName));
			currentBone.parent->offspring.push_back(&currentBone);

			linkToParentBone(parent, *currentBone.parent);	// Further recursion until a bone (or root node) is hit
		}
	}



	void propagateTransformations()
	{
		// Find root bone
		/*
		for (auto nameBone : _boneMap)
			if (nameBone.second.parent == nullptr)
				_root = nameBone.second;
		*/

		// Calculate transforms for all nodes
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



	inline bool boneExists(const std::string& name)
	{
		return ( _boneMap.find(name) != _boneMap.end() );
	}


	// Returns -1 if not found
	inline int getBoneIndex(const std::string& name)
	{
		auto found = _boneMap.find(name);
		return found != _boneMap.end() ? found->second.index : -1;
	}



	bool insertBone(const Bone& bone)
	{
		return (_boneMap.insert({ bone.name, bone }).second);
	}
};