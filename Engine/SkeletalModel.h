#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include <d3d11.h>

#include "assimp\Importer.hpp"	
#include "assimp\scene.h"
#include "assimp\postprocess.h" 
#include "SkeletalMesh.h"
#include "Animation.h"

class SkeletalModel {

public:

	std::vector<SkeletalMesh> meshes;
	std::vector<Animation> anims;
	std::vector<Texture> textures_loaded;

	std::string directory;
	std::string name;

	SMatrix transform;
	Joint rootJoint;

	SMatrix globalInverseTransform;

	///functions
	SkeletalModel() {}


	bool LoadModel(ID3D11Device* device, const std::string& path, float rUVx = 1, float rUVy = 1) {

		assert(fileExists(path) && "File does not exist! ...probably.");

		unsigned int pFlags = aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_FlipUVs |
			aiProcess_ConvertToLeftHanded;

		// Read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, pFlags);

		aiMatrix4x4 globInvTrans = scene->mRootNode->mTransformation;
		globInvTrans.Inverse();
		globalInverseTransform = SMatrix(&globInvTrans.a1);	//this might not work... probably won't as intended... try with decompose and reassemble if not

		// Check for errors
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr) {
			std::string errString("Assimp error:" + std::string(importer.GetErrorString()));
			OutputDebugStringA(errString.c_str());
			return false;
		}

		directory = path.substr(0, path.find_last_of('/'));
		name = path.substr(path.find_last_of('/') + 1, path.size());

		processNode(device, scene->mRootNode, scene, scene->mRootNode->mTransformation, rUVx, rUVy);

		loadAnimations(scene);
		return true;
	}



	// Processes a node in a recursive fashion. Processes each individual mesh located at the node  
	//and repeats this process on its children nodes (if any).
	bool processNode(ID3D11Device* device, aiNode* node, const aiScene* scene, aiMatrix4x4 parentTransform, float rUVx, float rUVy) {

		aiMatrix4x4 concatenatedTransform = parentTransform * node->mTransformation;	//or reversed! careful!
		// Process each mesh located at the current node
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			unsigned int ind = meshes.size();

			meshes.push_back(processSkeletalMesh(device, mesh, scene, ind, concatenatedTransform, rUVx, rUVy));
		}

		// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			this->processNode(device, node->mChildren[i], scene, concatenatedTransform, rUVx, rUVy);
		}
		return true;
	}




	//reads in vertices, indices and texture UVs of a mesh
	SkeletalMesh processSkeletalMesh(ID3D11Device* device, aiMesh *mesh, const aiScene *scene, unsigned int ind, aiMatrix4x4 parentTransform, float rUVx, float rUVy) {

		// Data to fill
		std::vector<BonedVert3D> vertices;
		std::vector<unsigned int> indices;
		std::vector<Joint> joints;
		std::vector<Texture> locTextures;
		

		bool hasTexCoords = false;
		if (mesh->mTextureCoords[0])
			hasTexCoords = true;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++) 
		{
			BonedVert3D vertex;

			//might have to transform these like so later... not sure
			//aiVector3D temp = parentTransform * aiVector3D(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			//vertex.pos = SVec3(temp.x, temp.y, temp.z);
			//aiVector3D tempNormals = parentTransform * aiVector3D(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			//vertex.normal = SVec3(tempNormals.x, tempNormals.y, tempNormals.z);

			//position, normal and texture coordinate data of each vertex
			vertex.pos = SVec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			vertex.normal = SVec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			vertex.normal.Normalize();

			if (hasTexCoords) // Does the mesh contain texture coordinates?
				vertex.texCoords = SVec2(mesh->mTextureCoords[0][i].x * rUVx, mesh->mTextureCoords[0][i].y * rUVy);
			else
				vertex.texCoords = SVec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}

		// Retrieve indices from faces...
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
				indices.push_back(mesh->mFaces[i].mIndices[j]);


		
		if (mesh->HasBones()) {
			std::vector<aiBone> bonesOfThisMesh;	//name, vertexWeight array (these are id + weight objects), offsetMatrix per bone


		}




		// Process materials
		if (mesh->mMaterialIndex >= 0) 
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			// 1. Diffuse maps
			std::vector<Texture> diffuseMaps = this->loadMaterialTextures(device, scene, material, aiTextureType_DIFFUSE, "texture_diffuse");
			locTextures.insert(locTextures.end(), diffuseMaps.begin(), diffuseMaps.end());
			// 2. Specular maps
			std::vector<Texture> specularMaps = this->loadMaterialTextures(device, scene, material, aiTextureType_SPECULAR, "texture_specular");
			locTextures.insert(locTextures.end(), specularMaps.begin(), specularMaps.end());

		}

		return SkeletalMesh(vertices, indices, locTextures, device, ind, joints);
	}



	std::vector<Texture> loadMaterialTextures(ID3D11Device* device, const aiScene* scene, aiMaterial *mat, aiTextureType type, std::string typeName) {

		std::vector<Texture> textures;

		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {

			aiString str;
			boolean skip = false;

			mat->GetTexture(type, i, &str);

			// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture			
			for (unsigned int j = 0; j < textures_loaded.size(); j++) {
				if (aiString(textures_loaded[j].fileName) == str) {
					textures.push_back(textures_loaded[j]);
					skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}

			if (!skip) {   // If texture hasn't been loaded already, load it

				std::string fPath = directory + "/" + std::string(str.data);
				Texture texture(device, fPath);
				texture.typeName = typeName;

				//texture.Bind(type);
				bool loaded = texture.Load();

				if (!loaded) {
					loaded = this->LoadGLTextures(device, textures, scene, fPath, type, typeName);	//for embedded textures

					if (!loaded)
						std::cout << "Texture did not load!" << std::endl;

					return textures;
				}

				textures.push_back(texture);
				textures_loaded.push_back(texture);
				// Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
			}
		}

		return textures;
	}



	bool LoadGLTextures(ID3D11Device* device, std::vector<Texture>& textures, const aiScene* scene, std::string& fPath, aiTextureType type, std::string& typeName) {

		if (scene->HasTextures()) {

			for (size_t ti = 0; ti < scene->mNumTextures; ti++) {

				Texture texture(device, fPath);
				texture.typeName = typeName;

				texture.LoadFromMemory(scene->mTextures[ti], device);

				textures.push_back(texture);
				textures_loaded.push_back(texture);
			}

			return true;
		}
		return false;
	}



	//void SkeletalModel::loadBones(const aiMesh& aiMesh, std::vector<BonedVert3D>& verts) 
	//{
	//	if (!aiMesh.HasBones())
	//		return;

	//	int numBones = 0;

	//	for (unsigned int i = 0; i < aiMesh.mNumBones; i++) 
	//	{
	//		aiBone* bone = aiMesh.mBones[i];	//bone at index i in assimp data, NOT THE INDEX WE ARE USING!!!
	//		int boneIndex = jointMap.size();		//index to be stored in my mesh data
	//		
	//		std::string boneName(bone->mName.data);

	//		if (jointMap.find(boneName) == jointMap.end()) {
	//			boneIndex = numBones;
	//			numBones++;
	//			//Joint bi;
	//			//m_BoneInfo.push_back(bi);
	//		}
	//		else {
	//			//boneIndex = jointMap[boneName];
	//		}

	//		Joint joint(boneIndex, boneName, SMatrix(bone->mOffsetMatrix[0]));
	//		jointMap.at(boneName) = joint;
	//	}
	//}



	void loadAnimations(const aiScene* scene) 
	{
		if (scene->HasAnimations())
			return;

		for (int i = 0; i < scene->mNumAnimations; i++) 
		{
			auto sceneAnimation = scene->mAnimations[i];
			Animation anim(std::string(sceneAnimation->mName.data), sceneAnimation->mDuration, sceneAnimation->mTicksPerSecond);

			for (int j = 0; j < sceneAnimation->mNumChannels; j++) 
			{
				aiNodeAnim* channel = sceneAnimation->mChannels[j];

				for (int a = 0; a < channel->mNumPositionKeys; a++) 
				{
					double time = channel->mPositionKeys[a].mTime;
					aiVector3D chPos = channel->mPositionKeys[a].mValue;
					SVec3 pos = SVec3(chPos.x, chPos.y, chPos.z);
				}

				for (int b = 0; b < channel->mNumRotationKeys; b++) 
				{
					double time = channel->mRotationKeys[b].mTime;
					aiQuaternion chRot = channel->mRotationKeys[b].mValue;
					SQuat rot = SQuat(chRot.x, chRot.y, chRot.z, chRot.w);
				}

				for (int c = 0; c < channel->mNumScalingKeys; c++) 
				{
					double time = channel->mScalingKeys[c].mTime;
					aiVector3D chScale = channel->mScalingKeys[c].mValue;
					SVec3 scale = SVec3(chScale.x, chScale.y, chScale.z);
				}
			}

			anims.push_back(anim);
		}
	}



	void Draw(ID3D11DeviceContext* dc, Animator& shader) {
		for (unsigned int i = 0; i < this->meshes.size(); i++)
			this->meshes[i].draw(dc, shader);
	}

private:

	inline bool fileExists(const std::string& name) {
		std::ifstream f(name.c_str());
		return f.good();
	}
};
