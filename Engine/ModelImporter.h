#pragma once
#include "Model.h"
#include "SkeletalModel.h"
#include "SkeletonImporter.h"
#include "AssimpWrapper.h"
#include "MatImporter.h"
#include "MeshImporter.h"


namespace ModelImporter
{

namespace
{
	template <typename AnyModelType>
	static void ProcessNode(aiNode* node, SMatrix modelSpaceTransform, AnyModelType& skModel)
	{
		SMatrix locNodeTransform = AssimpWrapper::aiMatToSMat(node->mTransformation);
		modelSpaceTransform = locNodeTransform * modelSpaceTransform;

		for (uint32_t i = 0; i < node->mNumMeshes; ++i)
		{
			skModel._meshes.at(node->mMeshes[i])._parentSpaceTransform = modelSpaceTransform;
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], modelSpaceTransform, skModel);
		}
	}
}

// Skeletal model import
static std::unique_ptr<SkeletalModel> ImportSkModelFromAiScene(ID3D11Device* device, const aiScene* scene, const std::string& path,
	std::vector<std::shared_ptr<Material>>& mats, std::shared_ptr<Skeleton>& skeleton)
{
	std::unique_ptr<SkeletalModel> skModel = std::make_unique<SkeletalModel>();

	skModel->_skeleton = skeleton;

	skModel->_meshes.reserve(scene->mNumMeshes);

	for (UINT i = 0; i < scene->mNumMeshes; ++i)
	{
		skModel->_meshes.emplace_back(MeshImporter::ImportFromAssimp(scene, device, scene->mMeshes[i], mats, skModel->_skeleton.get(), path));
		skModel->_meshes[i].setupMesh(device);
	}

	ProcessNode(scene->mRootNode, SMatrix::Identity, *skModel);

	return skModel;
}


static std::unique_ptr<SkeletalModel> StandaloneSkModelImport(ID3D11Device* device, const std::string& path, uint32_t extraFlags = 0u)
{
	std::unique_ptr<SkeletalModel> skModel = std::make_unique<SkeletalModel>();

	unsigned int pFlags =
		aiProcessPreset_TargetRealtime_MaxQuality |
		aiProcess_ConvertToLeftHanded |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_LimitBoneWeights;

	pFlags |= extraFlags;

	Assimp::Importer importer;

	const aiScene* scene = AssimpWrapper::loadScene(importer, path, pFlags);

	if (!scene)
	{
		return nullptr;
	}

	auto mats = MatImporter::ImportSceneMaterials(device, scene, path);
	std::shared_ptr<Skeleton> skeleton = SkeletonImporter::ImportSkeleton(scene);
	return ImportSkModelFromAiScene(device, scene, path, mats._materials, skeleton);
}


// Static model import
static std::unique_ptr<Model> ImportModelFromAiScene(ID3D11Device* device, const aiScene* scene, const std::string& path)
{
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->_meshes.reserve(scene->mNumMeshes);

	auto mats = MatImporter::ImportSceneMaterials(device, scene, path);

	for (UINT i = 0; i < scene->mNumMeshes; ++i)
	{
		model->_meshes.emplace_back();
		model->_meshes.back().loadFromAssimp(scene, device, scene->mMeshes[i], mats._materials, path);
		model->_meshes.back().setupMesh(device);
	}

	ProcessNode(scene->mRootNode, SMatrix::Identity, *model);

	return model;
}


static std::unique_ptr<Model> StandaloneModelImport(ID3D11Device* device, const std::string& path)
{
	assert(FileUtils::fileExists(path) && "File does not exist! ...probably.");

	UINT pFlags =
		aiProcessPreset_TargetRealtime_MaxQuality |
		aiProcess_ConvertToLeftHanded | 
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals;

	Assimp::Importer importer;

	const aiScene* scene = AssimpWrapper::loadScene(importer, path, pFlags);
	if (!scene)
	{
		return nullptr;
	}

	return ImportModelFromAiScene(device, scene, path);
}
}