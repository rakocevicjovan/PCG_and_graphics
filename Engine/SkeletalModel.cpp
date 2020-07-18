#include "SkeletalModel.h"



SkeletalModel::SkeletalModel()
{
}



SkeletalModel::~SkeletalModel()
{
}



bool SkeletalModel::loadModel(ID3D11Device* dvc, const std::string& path)
{
	_path = path;

	assert(FileUtils::fileExists(path) && "File not accessible!");

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
	else
		return loadFromAiScene(dvc, scene, path);
}



bool SkeletalModel::loadFromAiScene(ID3D11Device* device, const aiScene* scene, const std::string& path)
{
	_path = path;
	_meshes.reserve(scene->mNumMeshes);

	for (UINT i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh* aiMesh = scene->mMeshes[i];
		_meshes.emplace_back();
		_meshes.back().loadFromAssimp(scene, device, aiMesh, _skeleton, path);
		_meshes.back().setupSkeletalMesh(device);
	}

	SMatrix rootTransform = AssimpWrapper::aiMatToSMat(scene->mRootNode->mTransformation);
	_skeleton._globalInverseTransform = rootTransform.Invert();

	processNode(scene->mRootNode, SMatrix::Identity);

	_skeleton.loadFromAssimp(scene);

	AssimpWrapper::loadAnimations(scene, _anims);

	return true;
}



bool SkeletalModel::processNode(aiNode* node, SMatrix globNodeTransform)
{
	SMatrix locNodeTransform = AssimpWrapper::aiMatToSMat(node->mTransformation);
	globNodeTransform = locNodeTransform * globNodeTransform;

	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		_meshes[node->mMeshes[i]]._localTransform = globNodeTransform;
		_meshes[node->mMeshes[i]]._transform = globNodeTransform;
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
		this->processNode(node->mChildren[i], globNodeTransform);

	return true;
}