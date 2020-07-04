#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include <d3d11.h>
#include "AssimpWrapper.h"
#include "SkeletalMesh.h"
#include "AnimationInstance.h"
#include "Skeleton.h"
#include "SerializableAsset.h"



class SkeletalModel : public SerializableAsset
{
public:

	std::vector<SkeletalMesh> _meshes;

	std::string _path;

	SMatrix _transform;
	
	std::vector<Animation> _anims;

	Skeleton _skeleton;

	SkeletalModel();
	~SkeletalModel();


	bool loadModel(ID3D11Device* dvc, const std::string& path, float rUVx = 1, float rUVy = 1)
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
			return loadFromScene(dvc, scene, rUVx, rUVy);
	}



	bool loadFromScene(ID3D11Device* dvc, const aiScene* scene, float rUVx = 1, float rUVy = 1)
	{
		_meshes.reserve(scene->mNumMeshes);

		SMatrix rootTransform = AssimpWrapper::aiMatToSMat(scene->mRootNode->mTransformation);
		_skeleton._globalInverseTransform = rootTransform.Invert();

		processNode(dvc, scene->mRootNode, scene, rUVx, rUVy, SMatrix::Identity);

		_skeleton.loadFromAssimp(scene);

		AssimpWrapper::loadAnimations(scene, _anims);

		return true;
	}



	bool processNode(ID3D11Device* dvc, aiNode* node, const aiScene* scene, float rUVx, float rUVy, SMatrix globNodeTransform)
	{
		SMatrix locNodeTransform = AssimpWrapper::aiMatToSMat(node->mTransformation);
		globNodeTransform = locNodeTransform * globNodeTransform;
		
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
			_meshes.emplace_back(processSkeletalMesh(dvc, scene->mMeshes[node->mMeshes[i]], scene, _meshes.size(), globNodeTransform, rUVx, rUVy));

		// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
			this->processNode(dvc, node->mChildren[i], scene, rUVx, rUVy, globNodeTransform);

		return true;
	}



	SkeletalMesh processSkeletalMesh(ID3D11Device* device, aiMesh* mesh, const aiScene* scene, unsigned int ind, SMatrix transform, float rUVx, float rUVy)
	{
		// Data to fill
		std::vector<BonedVert3D> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> locTextures;

		bool hasTexCoords = mesh->HasTextureCoords(0);

		float radius = AssimpWrapper::loadVertices(mesh, hasTexCoords, vertices);

		AssimpWrapper::loadIndices(mesh, indices);

		AssimpWrapper::loadMaterials(_path, scene, mesh, locTextures);

		for (Texture& t : locTextures)
			t.SetUpAsResource(device);

		AssimpWrapper::loadBonesAndSkinData(*mesh, vertices, _skeleton, transform);
		// Elision does happen... I was wondering
		return SkeletalMesh(vertices, indices, locTextures, device, transform);
	}


	// This is wrong for now, need to see how to support asset aggregates
	MemChunk Serialize() override
	{
		std::vector<MemChunk> memChunks;

		memChunks.reserve(_meshes.size());

		for (int i = 0; i < _meshes.size(); ++i)
			memChunks.push_back(_meshes[i].Serialize());

		return std::move((memChunks[0]));
	}
};