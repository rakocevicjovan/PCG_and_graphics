#include "Skeleton.h"
#include "AssimpWrapper.h"


void Skeleton::makeLikeATree(const aiNode* node, SMatrix parentMatrix)	//PM NOT USED!!!
{
	std::string nodeName = std::string(node->mName.data);

	std::map<std::string, Bone>::iterator boneIterator = _boneMap.find(nodeName);

	SMatrix locNodeMat = AssimpWrapper::aiMatToSMat(node->mTransformation);

	parentMatrix = locNodeMat * parentMatrix;

	if (boneIterator != _boneMap.end())	// If node is a bone
	{
		Bone& currentBone = boneIterator->second;
		currentBone.localTransform = locNodeMat;
		linkToParentBone(node, currentBone);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		this->makeLikeATree(node->mChildren[i], parentMatrix);
}



void Skeleton::linkToParentBone(const aiNode* node, Bone& currentBone)
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
		SMatrix locNodeMat = AssimpWrapper::aiMatToSMat(parent->mTransformation);

		Bone newParentBone;
		newParentBone.name = parentName;
		newParentBone.index = _boneMap.size();
		newParentBone.localTransform = locNodeMat;
		_boneMap.insert({ parentName, newParentBone });

		// @TODO ADD INVERSE TRANSFORM TO THESE BONES, NOT CALCULATED YET!!!
		// WILL BREAK IF REQUIRED (LIKE ON WOLF MODEL!)

		currentBone.parent = &(_boneMap.at(parentName));
		currentBone.parent->offspring.push_back(&currentBone);

		linkToParentBone(parent, *currentBone.parent);	// Further recursion until a bone (or root node) is hit
	}
}



void Skeleton::calcGlobalTransforms(Bone& bone, const SMatrix& parentTransform)
{
	bone.globalTransform = bone.localTransform * parentTransform;

	for (Bone* childBone : bone.offspring)
	{
		calcGlobalTransforms(*childBone, bone.globalTransform);
	}
}