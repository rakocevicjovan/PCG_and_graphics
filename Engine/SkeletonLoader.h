#pragma once
#include "AssimpWrapper.h"



class SkeletonLoader
{
	Skeleton* _skeleton;

	std::set<aiBone*> _bones;

	std::set<aiNode*> _nodes;


	bool loadSkeleton(aiScene* scene)
	{
		//if (!AssimpWrapper::containsRiggedMeshes(scene)) return false;

		aiNode* rootNode = scene->mRootNode;

		findInfluenceBones(scene, rootNode, _bones);


		std::vector<aiNode*> temp(_bones.size());

		for (aiBone* bone : _bones)
		{
			temp.push_back(bone->mNode);
			_nodes.insert(bone->mNode);
		}

		for (aiNode* node : temp)
		{
			aiNode* parent = node->mParent;

			while (parent != nullptr)
			{
				if (parent->mParent != nullptr)
				{
					if (!_nodes.insert(parent).second)	// Exit if already added
						break;
				}
				parent = parent->mParent;
			}
		}

		aiNode* skelRoot = findSkeletonRoot(rootNode);

		if (!skelRoot)
			return false;

		return true;
	}



	void findInfluenceBones(aiScene* scene, aiNode* node, std::set<aiBone*>& boneSet)
	{
		for (int i = 0; i < node->mNumMeshes; ++i)	// Iterate through meshes in a node
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			for (UINT j = 0; j < mesh->mNumBones; ++j)	// For each bone referenced by the mesh
			{
				boneSet.insert(mesh->mBones[i]);
			}
		}

		for (UINT i = 0; i < node->mNumChildren; ++i)
		{
			findInfluenceBones(scene, node, boneSet);
		}
	}



	aiNode* findSkeletonRoot(aiNode* node)
	{
		aiNode* rootNode = nullptr;

		auto nodeFound = _nodes.find(node);
		if (nodeFound != _nodes.end())
		{
			rootNode = *nodeFound;
			return;
		}

		for (int i = 0; i < node->mNumChildren; ++i)
			findSkeletonRoot(node->mChildren[i]);
	}
};


/*
// Is this smart or dirty? We will never know... Got another solution though
	void traverseUp(std::set<aiNode*>& prev)
	{
		if (prev.size() == 0)	// 1. At some point, this returns
			return;

		std::set<aiNode*> current;

		for (aiNode* node : prev)
		{
			aiNode* parent = node->mParent;
			if(parent != nullptr)
				if(parent->mParent != nullptr)
					current.insert(node->mParent);
		}

		traverseUp(current);	// 2. Then we exit out of here
		prev.insert(current.begin(), current.end()); // 3. And merge into initial set
	}
*/