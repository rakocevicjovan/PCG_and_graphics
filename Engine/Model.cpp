#include "pch.h"
#include "Model.h"
#include "CollisionEngine.h"
#include "Terrain.h"
#include "FileUtilities.h"
#include "Collider.h"
#include "MatImporter.h"


Model::Model(const std::string& path) : collider(nullptr)
{
	_path = path;
}



Model::Model(const Procedural::Terrain& terrain, ID3D11Device* device) : collider(nullptr)
{
	_meshes.emplace_back(terrain, device);
	_transform = SMatrix::CreateTranslation(terrain.getOffset());
}



Model::~Model()
{
	if(collider) delete collider;
}



bool Model::loadFromAssimp(ID3D11Device* device, const std::string& path)
{
	_path = path;

	assert(FileUtils::fileExists(path) && "File does not exist! ...probably.");

	UINT pFlags =
		aiProcessPreset_TargetRealtime_MaxQuality |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs |
		aiProcess_PreTransformVertices |
		aiProcess_ConvertToLeftHanded;

	Assimp::Importer importer;

	const aiScene* scene = AssimpWrapper::loadScene(importer, path, pFlags);

	if (!scene)
		return false;

	return loadFromAiScene(device, scene, path);
}



bool Model::loadFromAiScene(ID3D11Device* device, const aiScene* scene, const std::string& path)
{
	_path = path;

	_meshes.reserve(scene->mNumMeshes);

	auto mats = MatImporter::ImportSceneMaterials(device, scene, _path);

	for (UINT i = 0; i < scene->mNumMeshes; ++i)
	{
		_meshes.emplace_back();
		_meshes.back().loadFromAssimp(scene, device, scene->mMeshes[i], mats._mats, _path);
		_meshes.back().setupMesh(device);
	}

	processNode(scene->mRootNode, SMatrix::Identity/* AssimpWrapper::aiMatToSMat(scene->mRootNode->mTransformation)*/);

	return true;
}



bool Model::processNode(aiNode* node, SMatrix modelSpaceTransform)
{
	SMatrix locNodeTransform = AssimpWrapper::aiMatToSMat(node->mTransformation);
	modelSpaceTransform = locNodeTransform * modelSpaceTransform;

	for (UINT i = 0; i < node->mNumMeshes; ++i)
	{
		_meshes[node->mMeshes[i]]._parentSpaceTransform = modelSpaceTransform;
	}

	for (UINT i = 0; i < node->mNumChildren; ++i)
	{
		processNode(node->mChildren[i], modelSpaceTransform);
	}

	return true;
}