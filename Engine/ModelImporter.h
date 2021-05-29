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
	static void ProcessNode(aiNode* node, SMatrix modelSpaceTransform, SkeletalModel& skModel)
	{
		SMatrix locNodeTransform = AssimpWrapper::aiMatToSMat(node->mTransformation);
		modelSpaceTransform = locNodeTransform * modelSpaceTransform;

		for (unsigned int i = 0; i < node->mNumMeshes; ++i)
		{
			skModel._meshes[node->mMeshes[i]]._parentSpaceTransform = modelSpaceTransform;
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], modelSpaceTransform, skModel);
		}
	}
}


static std::unique_ptr<SkeletalModel> ImportFromAiScene(ID3D11Device* device, const aiScene* scene, const std::string& path,
	std::vector<std::shared_ptr<Material>>& mats, std::shared_ptr<Skeleton>& skeleton)
{
	std::unique_ptr<SkeletalModel> skModel = std::make_unique<SkeletalModel>();

	skModel->_path = path;

	skModel->_skeleton = skeleton;

	skModel->_meshes.reserve(scene->mNumMeshes);

	for (UINT i = 0; i < scene->mNumMeshes; ++i)
	{
		skModel->_meshes.emplace_back(MeshImporter::ImportFromAssimp(scene, device, scene->mMeshes[i], mats, skModel->_skeleton.get(), path));
		skModel->_meshes[i].setupSkeletalMesh(device);
	}

	ProcessNode(scene->mRootNode, SMatrix::Identity, *skModel);

	return skModel;
}


static std::unique_ptr<SkeletalModel> StandaloneImport(ID3D11Device* device, const std::string& path, uint32_t extraFlags = 0u)
{
	std::unique_ptr<SkeletalModel> skModel = std::make_unique<SkeletalModel>();

	skModel->_path = path;

	unsigned int pFlags =
		aiProcessPreset_TargetRealtime_MaxQuality |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_ConvertToLeftHanded |
		aiProcess_LimitBoneWeights;

	pFlags |= extraFlags;

	Assimp::Importer importer;

	const aiScene* scene = AssimpWrapper::loadScene(importer, path, pFlags);

	if (!scene)
		return false;

	auto mats = MatImporter::ImportSceneMaterials(device, scene, path);
	std::shared_ptr<Skeleton> skeleton = SkeletonImporter::ImportSkeleton(scene);
	return ImportFromAiScene(device, scene, path, mats._materials, skeleton);
}

}