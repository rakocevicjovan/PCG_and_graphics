#include "Skeleton.h"
#include "AssimpWrapper.h"



void Skeleton::loadFromAssimp(const aiScene* scene, SMatrix meshOffset)
{
	for (auto namedBone : _boneMap)
	{
		aiNode* node = scene->mRootNode->FindNode(namedBone.first.c_str());
		addMissingBones(scene, node, namedBone.second);
	}

	const aiNode* skelRoot = AssimpWrapper::findSkeletonRoot(scene->mRootNode, *this, SMatrix());

	// Otherwise, skeleton is stored in another file and this is just a rigged model. Might happen? Not sure.
	if (skelRoot)
		linkSkeletonHierarchy(skelRoot);
}



void Skeleton::linkSkeletonHierarchy(const aiNode* skelRootNode)
{
	_root = findBone(skelRootNode->mName.C_Str());

	//skeleton.makeLikeATree(skelRootNode, SMatrix::Identity);

	// Skip root itself, it has a bit of a special transform (local IS global... my bad there)
	for (int i = 0; i < skelRootNode->mNumChildren; i++)
	{
		makeLikeATree(skelRootNode->mChildren[i], _root->_localMatrix);
	}

	calcGlobalTransforms(*_root, SMatrix::Identity);

	// This isn't supposed to be correct but it gives better results (where did I screw up?)
	_globalInverseTransform = _root->_localMatrix.Invert();
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


// Seeks upwards from every existing bone, filling in intermediate nodes
void Skeleton::addMissingBones(const aiScene* scene, const aiNode* childNode, Bone& childBone)
{
	aiNode* parent = childNode->mParent;

	// bones don't have local transforms yet so i use the node ones
	SMatrix childLocMat = AssimpWrapper::aiMatToSMat(childNode->mTransformation);

	if (!parent)			// We are at root node, no way but down
		return;

	if (!parent->mParent)	// Don't include the root node either... bit hacky but works out so far
		return;

	std::string parentName(parent->mName.C_Str());

	if (boneExists(parentName))	// Parent is already a bone, terminate
		return;

	Bone newParentBone;
	newParentBone.name = parentName;
	newParentBone.index = _boneMap.size();
	//newParentBone._offsetMatrix = childBone._offsetMatrix * childLocMat.Invert();
	auto boneIter = _boneMap.insert({ parentName, newParentBone });

	addMissingBones(scene, parent, boneIter.first->second);
}


/*
SMatrix Skeleton::calculateOffsetMatrix(SMatrix concat)
{
	string boneName = SkinInfo.GetBoneName(boneIndex);
	Frame boneFrame = FindFrame(root_frame, boneName);

	// error check for boneFrame == NULL 
	//if desired offsetMatrix[boneIndex] = MeshFrame.ToRoot * MatrixInverse(boneFrame.ToRoot);
	return (SMatrix::Identity * 
}
*/


void Skeleton::calcGlobalTransforms(Bone& bone, const SMatrix& parentTransform)
{
	bone._globalMatrix = bone._localMatrix * parentTransform;

	for (Bone* childBone : bone.offspring)
		calcGlobalTransforms(*childBone, bone._globalMatrix);
}
