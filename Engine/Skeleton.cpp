#include "Skeleton.h"
#include "AssimpWrapper.h"


// Assumes influencing bones were loaded already
void Skeleton::loadFromAssimp(const aiScene* scene)
{
	for (auto namedBone : _boneMap)
	{
		aiNode* boneNode = scene->mRootNode->FindNode(namedBone.first.c_str());
		AssimpWrapper::addMissingBones(this, boneNode, SMatrix::Identity);
	}

	const aiNode* skelRootNode = AssimpWrapper::findSkeletonRoot(scene->mRootNode, *this, SMatrix());

	// Otherwise, skeleton is stored in another file and this is just a rigged model. Might happen? Not sure.
	if (skelRootNode)
		linkSkeletonHierarchy(skelRootNode);
}



void Skeleton::loadStandalone(const aiScene* aiScene)
{
	AssimpWrapper::loadOnlySkeleton(aiScene->mRootNode, *this, SMatrix::Identity);
	_root = findBone(aiScene->mRootNode->mName.C_Str());
}



void Skeleton::linkSkeletonHierarchy(const aiNode* skelRootNode)
{
	_root = findBone(skelRootNode->mName.C_Str());

	// Skip root itself, it already contains a concatenated transform
	for (int i = 0; i < skelRootNode->mNumChildren; i++)
		makeLikeATree(_root, skelRootNode->mChildren[i], _root->_localMatrix);
}



void Skeleton::makeLikeATree(Bone* parent, const aiNode* node, SMatrix concat)
{
	std::string nodeName(node->mName.data);

	Bone* currentBone = findBone(nodeName);

	if (!currentBone)
		return;

	SMatrix locNodeMat = AssimpWrapper::aiMatToSMat(node->mTransformation);
	concat = locNodeMat * concat;

	currentBone->_localMatrix = locNodeMat;
	currentBone->_parent = parent;
	parent->_children.push_back(currentBone);

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		this->makeLikeATree(currentBone, node->mChildren[i], concat);
}



/*
// calcGlobalTransforms(*_root, SMatrix::Identity); in linkSkeletonHierarchy
// void calcGlobalTransforms(Bone& bone, const SMatrix& parentTransform); in header
void Skeleton::calcGlobalTransforms(Bone& bone, const SMatrix& parentTransform)
{
	bone._globalMatrix = bone._localMatrix * parentTransform;

	for (Bone* childBone : bone.offspring)
		calcGlobalTransforms(*childBone, bone._globalMatrix);
}*/
