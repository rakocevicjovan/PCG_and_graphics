#pragma once
#include "Model.h"
#include "MeshNode.h"
#include "SkeletalModel.h"
#include "SkeletonImporter.h"
#include "AssimpWrapper.h"
#include "MatImporter.h"
#include "MeshImporter.h"
#include "ModelAsset.h"


namespace ModelImporter
{

namespace
{
	static void ImportMeshNodeTree(const aiNode* node, SMatrix modelSpaceTransform, std::vector<MeshNode>& meshTree, uint16_t parentIndex = static_cast<uint16_t>(~0))
	{
		SMatrix locNodeTransform = AssimpWrapper::aiMatToSMat(node->mTransformation);
		modelSpaceTransform = locNodeTransform * modelSpaceTransform;

		if (node->mNumMeshes)
		{
			meshTree.push_back(MeshNode{ parentIndex, modelSpaceTransform, std::vector<uint16_t>{} });

			for (uint32_t i = 0; i < node->mNumMeshes; ++i)
			{
				meshTree.back().meshes.push_back(node->mMeshes[i]);
			}
		}

		uint16_t currentNodeIndex = meshTree.size() - 1;

		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ImportMeshNodeTree(node->mChildren[i], modelSpaceTransform, meshTree, currentNodeIndex);
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
		auto& aiMesh = scene->mMeshes[i];
		skModel->_meshes.emplace_back(MeshImporter::ImportFromAssimp(scene, device, aiMesh, mats[aiMesh->mMaterialIndex], skModel->_skeleton.get(), path));
		skModel->_meshes[i].setupMesh(device);
	}

	ImportMeshNodeTree(scene->mRootNode, SMatrix::Identity, skModel->_meshNodeTree);

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
static std::unique_ptr<Model> ImportModelFromAiScene(ID3D11Device* device, const aiScene* scene, const std::string& path, std::vector<std::shared_ptr<Material>>& mats)
{
	std::unique_ptr<Model> model = std::make_unique<Model>();
	ModelAsset modelAsset;

	auto& numMeshes = scene->mNumMeshes;

	model->_meshes.reserve(numMeshes);

	for (UINT i = 0; i < numMeshes; ++i)
	{
		auto& aiMesh = scene->mMeshes[i];
		uint32_t matIndex = aiMesh->mMaterialIndex;

		model->_meshes.emplace_back(MeshImporter::ImportFromAssimp(scene, device, aiMesh, mats[matIndex], nullptr, path));

		Mesh& mesh = model->_meshes.back();
		mesh.setupMesh(device);

		modelAsset.meshes.push_back(MeshAsset{ mesh._vertSig, mesh._vertices, mesh._indices, matIndex });

	}

	ImportMeshNodeTree(scene->mRootNode, SMatrix::Identity, model->_meshNodeTree);

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

	auto mats = MatImporter::ImportSceneMaterials(device, scene, path);
	return ImportModelFromAiScene(device, scene, path, mats._materials);
}
}