#include "SkeletalModel.h"
#include "MatLoader.h"


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

	auto mats = MatLoader::LoadAllMaterials(device, scene, _path);

	_meshes.reserve(scene->mNumMeshes);

	for (UINT i = 0; i < scene->mNumMeshes; ++i)
	{
		_meshes.emplace_back();

		aiMesh* aiMesh = scene->mMeshes[i];
		
		_meshes[i].loadFromAssimp(scene, device, aiMesh, mats, *_skeleton, path);
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