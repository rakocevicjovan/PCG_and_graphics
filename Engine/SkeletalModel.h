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



class SkeletalModel
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

		importer.SetPropertyInteger(AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, 0);

		const aiScene* scene = AssimpWrapper::loadScene(importer, path, pFlags);

		if (!scene)
			return false;
		else
			return loadFromScene(dvc, scene, rUVx, rUVy);
	}



	bool loadFromScene(ID3D11Device* dvc, const aiScene* scene, float rUVx = 1, float rUVy = 1)
	{
		_meshes.reserve(scene->mNumMeshes);

		SMatrix globalTransform = AssimpWrapper::aiMatToSMat(scene->mRootNode->mTransformation);
		_skeleton._globalInverseTransform = globalTransform.Invert();

		processNode(dvc, scene->mRootNode, scene, rUVx, rUVy, SMatrix::Identity);

		// This won't check "indirect" bones, breaks on wolf example (needs to be solved before this)
		const aiNode* skelRoot = AssimpWrapper::findSkeletonRoot(scene->mRootNode, _skeleton, SMatrix());

		// Otherwise, skeleton is stored in another file and this is just a rigged model. Might happen? Not sure.
		if (skelRoot)
			AssimpWrapper::linkSkeletonHierarchy(skelRoot, _skeleton);

		AssimpWrapper::loadAnimations(scene, _anims);

		return true;
	}



	bool processNode(ID3D11Device* dvc, aiNode* node, const aiScene* scene, float rUVx, float rUVy, SMatrix parentMat)	//aiMatrix4x4 parentTransform, 
	{
		SMatrix locNodeTransform = AssimpWrapper::aiMatToSMat(node->mTransformation);
		parentMat = locNodeTransform * parentMat;
		
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
			_meshes.push_back(processSkeletalMesh(dvc, scene->mMeshes[node->mMeshes[i]], scene, _meshes.size(), parentMat, rUVx, rUVy)); /*concatenatedTransform*/

		// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
			this->processNode(dvc, node->mChildren[i], scene, rUVx, rUVy, parentMat);

		return true;
	}



	SkeletalMesh processSkeletalMesh(ID3D11Device* device, aiMesh *mesh, const aiScene *scene, unsigned int ind, SMatrix transform, float rUVx, float rUVy)
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

		AssimpWrapper::loadBonesAndSkinData(*mesh, vertices, _skeleton);

		return SkeletalMesh(vertices, indices, locTextures, device, ind, transform);
	}

};



class SkeletalModelInstance
{
public:

	CBuffer _skeletonTransforms;
	std::vector<SMatrix> _skeletonMatrices;

	SkeletalModel* _skm;

	std::vector<AnimationInstance> _animInstances;

	SkeletalModelInstance() : _skm(nullptr) {}

	bool init(ID3D11Device* dvc, SkeletalModel* skm)
	{
		_skm = skm;

		for (Animation& anim : skm->_anims)
		{
			_animInstances.emplace_back(anim);
		}

		
		D3D11_BUFFER_DESC desc = ShaderCompiler::createBufferDesc(sizeof(SMatrix) * 96);
		_skeletonMatrices.resize(_skm->_skeleton._boneMap.size());

		if (FAILED(dvc->CreateBuffer(&desc, NULL, &_skeletonTransforms._cbPtr)))
			return false;

		return true;
	}



	void update(float dTime, UINT animIndex = 0u)
	{
		for (int i = 0; i < _animInstances.size(); ++i)
			_animInstances[i].update(dTime);

		for (int i = 0; i < _skm->_meshes.size(); ++i)
		{
			_skm->_meshes[i]._transform = _skm->_meshes[i]._localTransform * _skm->_transform;
		}

		_animInstances[animIndex].getTransformAtTime(*_skm->_skeleton._root, _skeletonMatrices, SMatrix::Identity, _skm->_skeleton._globalInverseTransform);
	}



	void draw(ID3D11DeviceContext* context)
	{
		_skeletonTransforms.updateWholeBuffer(
			context, _skeletonTransforms._cbPtr, _skeletonMatrices.data(), sizeof(SMatrix) * _skm->_skeleton._boneMap.size());

		context->VSSetConstantBuffers(1, 1, &_skeletonTransforms._cbPtr);

		for (SkeletalMesh& m : _skm->_meshes)
		{
			m.draw(context);
		}
	}
};