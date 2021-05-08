#pragma once
#include "AssimpWrapper.h"
#include "Skeleton.h"
#include <queue>
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
			aiNode* aiParent = node->mParent;

			while (aiParent != nullptr)
			{
				if (aiParent->mParent != nullptr)
				{
					if (!boneNodes.insert(aiParent).second)	// Exit if parent is already added to the set as an optimization
						break;
				}
				aiParent = aiParent->mParent;
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


	static void LinearizeBFT(aiNode* root, Skeleton* skeleton, uint16_t parentIndex, const std::set<aiNode*>& boneNodes, const std::set<aiBone*>& bones)
	{
		std::queue<std::pair<aiNode*, BoneIndex>> _nodesToProcess;

		auto& boneVec = skeleton->_bones;

		_nodesToProcess.push({ root, Bone::INVALID_INDEX });

		// Fix parent indices by pairing here
		while (!_nodesToProcess.empty())
		{
			auto [node, parentIndex] = _nodesToProcess.front();
			_nodesToProcess.pop();

			// All bones are expected to form a fully reachable tree at this point so we can make this optimization.
			// Nodes that do not belong to the boneNodes set are not bones, and their children will not be either - therefore stop traversing the tree. 
			if (boneNodes.count(node) == 0)
			{
				continue;
			}

			Bone bone(node->mName.C_Str(), AssimpWrapper::aiMatToSMat(node->mTransformation), parentIndex);

			// Set child offset for parent. We know children will be contiguous, and their count, when the parent is created, so that's already set.
			if (parentIndex != Bone::INVALID_INDEX)
			{
				auto& [childrenOffset, childrenCount] = boneVec[parentIndex]._children;
				
				if (childrenOffset == Bone::INVALID_INDEX)
				{
					childrenOffset = boneVec.size();
				}
				++childrenCount;
			}

			for (aiBone* aiBone : bones)
			{
				if (aiBone->mNode == node)
				{
					bone._isInfluenceBone = true;
					bone._invBindPose = AssimpWrapper::aiMatToSMat(aiBone->mOffsetMatrix);
					++(skeleton->_numInfluenceBones);
					break;
				}
			}

			boneVec.push_back(std::move(bone));

			for (UINT i = 0; i < node->mNumChildren; ++i)
			{
				_nodesToProcess.push({ node->mChildren[i], boneVec.size() - 1 });
			}
		}
	}


	static void LoadNodesAsBones(aiNode* node, std::vector<Bone>& bones, uint16_t parentIndex)
	{
		// Offset matrices are not present, they depend on the model we attach this to and can not be calculated...
		bones.push_back(Bone(node->mName.C_Str(), AssimpWrapper::aiMatToSMat(node->mTransformation), parentIndex));

		for (UINT i = 0; i < node->mNumChildren; ++i)
		{
			LoadNodesAsBones(node->mChildren[i], bones, bones.size() - 1);
		}
	}


public:

	static std::unique_ptr<Skeleton> ImportSkeleton(const aiScene* scene)
	{
		std::set<aiBone*> influenceBones;
		std::set<aiNode*> boneNodes;

		aiNode* sceneRoot = scene->mRootNode;

		FindInfluenceBones(scene, influenceBones);

		FindAllBoneNodes(influenceBones, boneNodes);

		// Find the closest aiNode to the root
		aiNode* skelRoot = FindSkeletonRootNode(sceneRoot, boneNodes);

		if (!skelRoot)
			return nullptr;

		std::unique_ptr<Skeleton> skeleton = std::make_unique<Skeleton>();
		skeleton->_bones.reserve(boneNodes.size());

		// Remember to split influence bones from non influence bones
		LinearizeBFT(skelRoot, skeleton.get(), Bone::INVALID_INDEX, boneNodes, influenceBones);

		// I NEVER SET THE SKELETON INVERSE GLOBAL TRANSFORM! INVESTIGATE WHY!

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