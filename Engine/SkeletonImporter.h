#pragma once
#include "AssimpWrapper.h"
#include "Skeleton.h"
#include <stack>
#include <memory>


class SkeletonImporter
{
private:

	// Gather all bones directly influencing the vertices on each and every mesh in the scene.
	static void FindInfluenceBones(const aiScene* scene, std::set<aiBone*>& boneSet)
	{
		for (UINT i = 0; i < scene->mNumMeshes; ++i)
		{
			aiMesh* mesh = scene->mMeshes[i];

			for (UINT j = 0; j < mesh->mNumBones; ++j)
			{
				if (mesh->mBones[j])
				{
					boneSet.insert(mesh->mBones[j]);
				}
			}
		}
	}

	// Find all aiNodes that are actually bones, by tracking upwards towards the scene root from every influencing bone.
	static void FindAllBoneNodes(const std::set<aiBone*>& aiBones, std::set<aiNode*>& boneNodes)
	{
		std::set<aiNode*> influencingNodes;

		for (aiBone* aiBone : aiBones)
		{
			influencingNodes.insert(aiBone->mNode);
			boneNodes.insert(aiBone->mNode);
		}

		for (aiNode* node : influencingNodes)
		{
			aiNode* parent = node->mParent;

			while (parent != nullptr)
			{
				if (parent->mParent != nullptr)
				{
					if (!boneNodes.insert(parent).second)	// Exit if parent is already added to the set as an optimization
						break;
				}
				parent = parent->mParent;
			}
		}
	}


	static aiNode* FindSkeletonRootNode(aiNode* node, const std::set<aiNode*>& boneNodes)
	{
		aiNode* result = nullptr;

		auto nodeFound = boneNodes.find(node);

		if (nodeFound != boneNodes.end())
		{
			result = *nodeFound;
		}
		else
		{
			for (UINT i = 0; i < node->mNumChildren; ++i)
			{
				result = FindSkeletonRootNode(node->mChildren[i], boneNodes);
				if (result)
					break;
			}
		}

		return result;
	}


	static void MakeLikeATree(aiNode* node, std::vector<Bone>& boneVec, uint16_t parentIndex,
		const std::set<aiNode*>& boneNodes, const std::set<aiBone*>& bones)
	{
		// All bones are expected to form a fully reachable tree at this point so we can make this optimization.
		// Nodes that do not belong to the boneNodes set are not bones, and their children will not be either - therefore stop traversing the tree. 
		if (boneNodes.count(node) == 0)
		{
			return;
		}

		auto boneIndex = boneVec.size();

		Bone bone(boneIndex, node->mName.C_Str(), AssimpWrapper::aiMatToSMat(node->mTransformation));
		bone._parent = parentIndex;

		// A bit slow but this is offline and the sets are usually fairly small
		for (aiBone* aiBone : bones)
		{
			if (aiBone->mNode == node)
			{
				bone._offsetMatrix = AssimpWrapper::aiMatToSMat(aiBone->mOffsetMatrix);
			}
		}

		boneVec.push_back(bone);

		if (bone._parent != Bone::INVALID_INDEX)
		{
			boneVec[bone._parent]._children.push_back(boneIndex);
		}

		for (UINT i = 0; i < node->mNumChildren; ++i)
		{
			MakeLikeATree(node->mChildren[i], boneVec, boneIndex, boneNodes, bones);
		}
	}


	static void LoadNodesAsBones(aiNode* node, std::vector<Bone>& bones, uint16_t parentIndex)
	{
		Bone b(bones.size(), node->mName.C_Str(), AssimpWrapper::aiMatToSMat(node->mTransformation));
		b._parent = parentIndex;
		// Offset matrices are not present, they depend on the model we attach this to and can not be calculated...
		bones.push_back(b);
		
		if (parentIndex != Bone::INVALID_INDEX)
		{
			bones[parentIndex]._children.push_back(b._index);
		}

		for (UINT i = 0; i < node->mNumChildren; ++i)
		{
			LoadNodesAsBones(node->mChildren[i], bones, b._index);
		}
	}


public:

	static std::unique_ptr<Skeleton> ImportSkeleton(const aiScene* scene)
	{
		std::set<aiNode*> boneNodes;
		std::set<aiBone*> bones;

		aiNode* sceneRoot = scene->mRootNode;

		FindInfluenceBones(scene, bones);

		FindAllBoneNodes(bones, boneNodes);

		// Find the closest aiNode to the root
		aiNode* skelRoot = FindSkeletonRootNode(sceneRoot, boneNodes);

		if (!skelRoot)
			return nullptr;

		std::unique_ptr<Skeleton> skeleton = std::make_unique<Skeleton>();
		skeleton->_bones.reserve(boneNodes.size());

		MakeLikeATree(skelRoot, skeleton->_bones, Bone::INVALID_INDEX, boneNodes, bones);

		// This might need to be done theoretically, fixes bee, breaks bobbert... idk what to do, think formats/assimp are inconsistent.
		/*
		aiNode* temp = skelRoot;
		SMatrix rootMatrix = SMatrix::Identity;
		while (temp)
		{
			rootMatrix *= AssimpWrapper::aiMatToSMat(temp->mTransformation);
			temp = temp->mParent;
		}

		skeleton->_bones[0]._localMatrix = rootMatrix;
		*/

		return skeleton;
	}


	static std::unique_ptr<Skeleton> loadStandalone(const aiScene* scene)
	{
		std::unique_ptr<Skeleton> skelly = std::make_unique<Skeleton>();

		UINT boneCount = AssimpWrapper::countChildren(scene->mRootNode) + 1;
		
		skelly->_bones.reserve(boneCount);

		LoadNodesAsBones(scene->mRootNode, skelly->_bones, Bone::INVALID_INDEX);

		return skelly;
	}
};