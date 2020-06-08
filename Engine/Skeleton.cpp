#include "Skeleton.h"
#include "AssimpWrapper.h"


// Assumes some bones were loaded already
void Skeleton::loadFromAssimp(const aiScene* scene)
{
	for (auto namedBone : _boneMap)
	{
		aiNode* boneNode = scene->mRootNode->FindNode(namedBone.first.c_str());
		AssimpWrapper::addMissingBones(*this, boneNode, SMatrix::Identity);
	}

	const aiNode* skelRoot = AssimpWrapper::findSkeletonRoot(scene->mRootNode, *this, SMatrix());

	// Otherwise, skeleton is stored in another file and this is just a rigged model. Might happen? Not sure.
	if (skelRoot)
		linkSkeletonHierarchy(skelRoot);
}



void Skeleton::linkSkeletonHierarchy(const aiNode* skelRootNode)
{
	_root = findBone(skelRootNode->mName.C_Str());

	// Skip root itself, it has a bit of a special transform (local IS global... my bad there)
	for (int i = 0; i < skelRootNode->mNumChildren; i++)
	{
		makeLikeATree(skelRootNode->mChildren[i], _root->_localMatrix);
	}

	//calcGlobalTransforms(*_root, SMatrix::Identity);
}


// Overwrites root's transform...
void Skeleton::makeLikeATree(const aiNode* node, SMatrix concat)
{
	std::string nodeName(node->mName.data);

	std::map<std::string, Bone>::iterator boneIterator = _boneMap.find(nodeName);

	SMatrix locNodeMat = AssimpWrapper::aiMatToSMat(node->mTransformation);

	concat = locNodeMat * concat;

	if (boneIterator != _boneMap.end())	// If node is a bone
	{
		Bone& currentBone = boneIterator->second;
		currentBone._localMatrix = locNodeMat;
		linkToParentBone(node, currentBone);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		this->makeLikeATree(node->mChildren[i], concat);
}



// Concatenated matrix represents the global transform of a child node we are currently coming from
void Skeleton::linkToParentBone(const aiNode* node, Bone& currentBone)
{
	aiNode* parent = node->mParent;

	if (parent == nullptr)	// We are at root node
		return;

	std::string parentName(node->mParent->mName.data);

	auto existingBone = _boneMap.find(parentName);

	if (existingBone != _boneMap.end())	// Found it, no need to go further
	{
		currentBone.parent = &(existingBone->second);
		currentBone.parent->offspring.push_back(&currentBone);
	}
}



/*void Skeleton::calcGlobalTransforms(Bone& bone, const SMatrix& parentTransform)
{
	bone._globalMatrix = bone._localMatrix * parentTransform;

	for (Bone* childBone : bone.offspring)
		calcGlobalTransforms(*childBone, bone._globalMatrix);
}*/
