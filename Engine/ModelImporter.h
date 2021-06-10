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
	template <typename ModelType>
	struct ModelImportData
	{
		std::unique_ptr<ModelType> model;
		std::vector<uint32_t> meshMaterialMapping;

		operator bool() { return model.get(); }
	};

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
static ModelImportData<SkModel> ImportSkModelFromAiScene(ID3D11Device* device, const aiScene* scene, const std::string& path,
	std::vector<std::shared_ptr<Material>>& mats, std::shared_ptr<Skeleton>& skeleton)
{
	std::unique_ptr<SkModel> skModel = std::make_unique<SkModel>();

	skModel->_skeleton = skeleton;

	const auto numMeshes = scene->mNumMeshes;

	std::vector<uint32_t> meshMatMapping(numMeshes);
	skModel->_meshes.reserve(numMeshes);

	for (UINT i = 0; i < numMeshes; ++i)
	{
		auto& aiMesh = scene->mMeshes[i];

		skModel->_meshes.emplace_back(MeshImporter::ImportFromAssimp(scene, device, aiMesh, mats[aiMesh->mMaterialIndex], skModel->_skeleton.get(), path));
		skModel->_meshes[i].setupMesh(device);	// Probably should move ALL this gpu related code out of here, would be a really good idea...

		meshMatMapping[i] = aiMesh->mMaterialIndex;
	}

	ImportMeshNodeTree(scene->mRootNode, SMatrix::Identity, skModel->_meshNodeTree);

	return { std::move(skModel), std::move(meshMatMapping)};
}


static ModelImportData<SkModel> StandaloneSkModelImport(ID3D11Device* device, const std::string& path, uint32_t extraFlags = 0u)
{
	std::unique_ptr<SkModel> skModel = std::make_unique<SkModel>();

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
		return { nullptr, {} };
	}

	auto mats = MatImporter::ImportSceneMaterials(device, scene, path);
	std::shared_ptr<Skeleton> skeleton = SkeletonImporter::ImportSkeleton(scene);
	return ImportSkModelFromAiScene(device, scene, path, mats._materials, skeleton);
}


// Static model import
static  ModelImportData<Model> ImportModelFromAiScene(ID3D11Device* device, const aiScene* scene, const std::string& path, std::vector<std::shared_ptr<Material>>& mats)
{
	std::unique_ptr<Model> model = std::make_unique<Model>();

	auto& numMeshes = scene->mNumMeshes;

	std::vector<uint32_t> meshMatMapping(numMeshes);
	model->_meshes.reserve(numMeshes);

	for (UINT i = 0; i < numMeshes; ++i)
	{
		auto& aiMesh = scene->mMeshes[i];

		model->_meshes.emplace_back(MeshImporter::ImportFromAssimp(scene, device, aiMesh, mats[aiMesh->mMaterialIndex], nullptr, path));
		model->_meshes[i].setupMesh(device);

		meshMatMapping[i] = aiMesh->mMaterialIndex;
	}

	ImportMeshNodeTree(scene->mRootNode, SMatrix::Identity, model->_meshNodeTree);

	return { std::move(model), std::move(meshMatMapping) };
}


static ModelImportData<Model> StandaloneModelImport(ID3D11Device* device, const std::string& path)
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
		return { nullptr, {} };
	}

	auto mats = MatImporter::ImportSceneMaterials(device, scene, path);
	return ImportModelFromAiScene(device, scene, path, mats._materials);
}

}