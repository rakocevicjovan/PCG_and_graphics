#include "pch.h"
#include "SkeletalModel.h"
#include "SkeletonLoader.h"
#include "MatLoader.h"


// Used for loading on the fly, preferably for very rare cases
bool SkeletalModel::importFromFileAssimp(ID3D11Device* device, const std::string& path)
{
	_path = path;

	unsigned int pFlags =
		aiProcessPreset_TargetRealtime_MaxQuality |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_ConvertToLeftHanded |
		aiProcess_LimitBoneWeights;

	Assimp::Importer importer;

	const aiScene* scene = AssimpWrapper::loadScene(importer, path, pFlags);

	if (!scene)
		return false;

	auto mats = MatLoader::LoadAllMaterials(device, scene, _path);
	std::shared_ptr<Skeleton> skeleton = SkeletonLoader::loadSkeleton(scene);
	return importFromAiScene(device, scene, path, mats._mats, skeleton);
}



bool SkeletalModel::importFromAiScene(ID3D11Device* device, const aiScene* scene, const std::string& path, 
	std::vector<std::shared_ptr<Material>>& mats, std::shared_ptr<Skeleton>& skeleton)
{
	_path = path;

	_skeleton = skeleton;

	_meshes.reserve(scene->mNumMeshes);

	for (UINT i = 0; i < scene->mNumMeshes; ++i)
	{
		_meshes.emplace_back();
		_meshes[i].loadFromAssimp(scene, device, scene->mMeshes[i], mats, _skeleton.get(), path);
		_meshes[i].setupSkeletalMesh(device);
	}

	// Not sure why is it inverted in tutorials I've seen
	//SMatrix rootTransform = AssimpWrapper::aiMatToSMat(scene->mRootNode->mTransformation);
	//rootTransform = rootTransform.Invert();

	processNode(scene->mRootNode, SMatrix::Identity);

	return true;
}



bool SkeletalModel::processNode(aiNode* node, SMatrix globNodeTransform)
{
	SMatrix locNodeTransform = AssimpWrapper::aiMatToSMat(node->mTransformation);
	globNodeTransform = locNodeTransform * globNodeTransform;

	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		_meshes[node->mMeshes[i]]._localTransform = globNodeTransform;
		//_meshes[node->mMeshes[i]]._transform = globNodeTransform;
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
		this->processNode(node->mChildren[i], globNodeTransform);

	return true;
}