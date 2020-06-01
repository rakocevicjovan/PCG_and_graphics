#include "Skeleton.h"
#include "AssimpWrapper.h"



void Skeleton::loadFromAssimp(const aiScene* scene)
{
	for (auto namedBone : _boneMap)
	{
		aiNode* node = scene->mRootNode->FindNode(namedBone.first.c_str());
		addMissingBones(scene, node);
	}

	const aiNode* skelRoot = AssimpWrapper::findSkeletonRoot(scene->mRootNode, *this, SMatrix());

	// Otherwise, skeleton is stored in another file and this is just a rigged model. Might happen? Not sure.
	if (skelRoot)
		AssimpWrapper::linkSkeletonHierarchy(skelRoot, *this);
}



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
		linkToParentBone(node, currentBone, concat);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		this->makeLikeATree(node->mChildren[i], concat);
}



// Concatenated matrix represents the global transform of a child node we are currently coming from
void Skeleton::linkToParentBone(const aiNode* node, Bone& currentBone, SMatrix concat)
{
	aiNode* parent = node->mParent;

	if (parent == nullptr)	// We are at root node
		return;

	if (parent->mParent == nullptr)	// Exclude the root node as well
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
		newParentBone._localMatrix = locNodeMat;

		// @TODO ADD INVERSE TRANSFORM TO THESE BONES, NOT CALCULATED YET!!!
		// WILL BREAK IF REQUIRED (LIKE ON WOLF MODEL!)

		_boneMap.insert({ parentName, newParentBone });

		currentBone.parent = &(_boneMap.at(parentName));
		currentBone.parent->offspring.push_back(&currentBone);

		// Further recursion until a bone (or root node) is hit
		linkToParentBone(parent, *currentBone.parent, concat);
	}
}


// Seeks upwards from every existing bone, filling in intermediate nodes
void Skeleton::addMissingBones(const aiScene* scene, const aiNode* node)
{
	aiNode* parent = node->mParent;

	if (!parent)			// We are at root node, no way but down
		return;

	if (!parent->mParent)	// Don't include the root node either... bit hacky but ok
		return;

	std::string parentName(parent->mName.C_Str());

	if (boneExists(parentName))	// Parent is already a bone, terminate
		return;

	Bone newParentBone;
	newParentBone.name = parentName;
	newParentBone.index = _boneMap.size();
	_boneMap.insert({ parentName, newParentBone });

	addMissingBones(scene, parent);
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

/*
void Skeleton::calcGlobalTransforms(Bone& bone, const SMatrix& parentTransform)
{
	bone.globalTransform = bone._localMatrix * parentTransform;

	for (Bone* childBone : bone.offspring)
		calcGlobalTransforms(*childBone, bone.globalTransform);
}
*/