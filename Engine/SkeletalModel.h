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


//@TODO Change code to use assimp wrapper
class SkeletalModel
{
public:

	std::vector<SkeletalMesh> _meshes;
	std::vector<Texture> textures_loaded;

	std::string directory;
	std::string name;

	SMatrix transform;
	
	std::vector<Animation> anims;

	Skeleton _skeleton;

	std::vector<AnimationInstance> _animInstances;


	SkeletalModel();
	~SkeletalModel();


	bool loadModel(ID3D11Device* dvc, const std::string& path, float rUVx = 1, float rUVy = 1)
	{
		assert(FileUtils::fileExists(path) && "File does not exist! ...probably.");

		unsigned int pFlags = aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_FlipUVs |
			aiProcess_ConvertToLeftHanded |
			aiProcess_LimitBoneWeights;

		// Read file via ASSIMP
		Assimp::Importer importer;

		const aiScene* scene = AssimpWrapper::loadScene(importer, path, pFlags);

		directory = path.substr(0, path.find_last_of('/'));
		name = path.substr(path.find_last_of('/') + 1, path.size());

		aiMatrix4x4 globInvTrans = scene->mRootNode->mTransformation;
		_skeleton._globalInverseTransform = SMatrix(&globInvTrans.a1).Transpose().Invert();

		processNode(dvc, scene->mRootNode, scene, rUVx, rUVy);	/*scene->mRootNode->mTransformation*/

		//adds parent/child relationships
		//relies on names to detect bones amongst other nodes (processNode already collected all bone names using loadBones)
		//and then on map searches to find relationships between the bones
		_skeleton.makeLikeATree(scene->mRootNode);	
		_skeleton.propagateTransformations();
		AssimpWrapper::loadAnimations(scene, anims);

		return true;
	}



	bool processNode(ID3D11Device* dvc, aiNode* node, const aiScene* scene, float rUVx, float rUVy)	//aiMatrix4x4 parentTransform, 
	{
		//aiMatrix4x4 concatenatedTransform = parentTransform * node->mTransformation;	//or reversed! careful!
		
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
			_meshes.push_back(processSkeletalMesh(dvc, scene->mMeshes[node->mMeshes[i]], scene, _meshes.size(), rUVx, rUVy)); /*concatenatedTransform*/

		// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
			this->processNode(dvc, node->mChildren[i], scene, rUVx, rUVy);

		return true;
	}



	SkeletalMesh processSkeletalMesh(ID3D11Device* device, aiMesh *mesh, const aiScene *scene, unsigned int ind/*, aiMatrix4x4 parentTransform*/, float rUVx, float rUVy)
	{
		// Data to fill
		std::vector<BonedVert3D> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> locTextures;

		bool hasTexCoords = mesh->HasTextureCoords(0);

		float radius = AssimpWrapper::loadVertices(mesh, hasTexCoords, vertices);

		AssimpWrapper::loadIndices(mesh, indices);

		AssimpWrapper::loadMaterials(directory, scene, mesh, locTextures);

		for (Texture& t : locTextures)
			t.SetUpAsResource(device);

		AssimpWrapper::loadBonesAndSkinData(*mesh, vertices, _skeleton);

		return SkeletalMesh(vertices, indices, locTextures, device, ind);
	}



	void update(float dTime, std::vector<SMatrix>& vec, UINT animIndex = 0u)
	{
		for (int i = 0; i < _animInstances.size(); ++i)
			_animInstances[i].update(dTime);

		_animInstances[animIndex].getTransformAtTime(_skeleton._root, vec, SMatrix::Identity, _skeleton._globalInverseTransform);
	}
};