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


//@TODO Make this use the model class for everything but skinning... 
//can't replicate all material changes that happened without messing something up
class SkeletalModel
{
public:

	std::vector<SkeletalMesh> meshes;
	std::vector<Texture> textures_loaded;

	std::string directory;
	std::string name;

	SMatrix transform;
	
	std::vector<Animation> anims;
	std::map<std::string, Joint> _boneMap;
	Joint _rootJoint;
	SMatrix _globalInverseTransform;

	///functions
	SkeletalModel();
	~SkeletalModel();


	bool loadModel(ID3D11Device* dvc, const std::string& path, float rUVx = 1, float rUVy = 1)
	{
		assert(fileExists(path) && "File does not exist! ...probably.");

		unsigned int pFlags = aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_FlipUVs |
			aiProcess_ConvertToLeftHanded |
			aiProcess_LimitBoneWeights;

		// Read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, pFlags);

		aiMatrix4x4 globInvTrans = scene->mRootNode->mTransformation;
		_globalInverseTransform = SMatrix(&globInvTrans.a1);
		_globalInverseTransform = _globalInverseTransform.Transpose().Invert();

		// Check for errors
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
		{
			std::string errString("Assimp error:" + std::string(importer.GetErrorString()));
			OutputDebugStringA(errString.c_str());
			return false;
		}

		directory = path.substr(0, path.find_last_of('/'));
		name = path.substr(path.find_last_of('/') + 1, path.size());

		processNode(dvc, scene->mRootNode, scene, rUVx, rUVy);	/*scene->mRootNode->mTransformation*/

		//adds parent/child relationships
		//relies on names to detect bones amongst other nodes (processNode already collected all bone names using loadBones)
		//and then on map searches to find relationships between the bones
		linkSkeletonTree(scene->mRootNode);	
		MakeLikeATree();
		loadAnimations(scene);

		//auto it = std::find_if(_boneMap.begin(), _boneMap.end(), [](const std::pair<std::string, Joint>& njPair) { return njPair.second.index == 10; });
		//Joint foundJoint = it->second;

		return true;
	}



	void linkSkeletonTree(const aiNode* node)
	{
		auto it = _boneMap.find(std::string(node->mName.data));

		//if is a bone
		if (it != _boneMap.end())
		{
			Joint& currentJoint = it->second;
			currentJoint.aiNodeTransform = (SMatrix(&node->mTransformation.a1).Transpose());

			if (node->mParent != nullptr)
			{
				auto it2 = _boneMap.find(std::string(node->mParent->mName.data));

				//currentJoint.aiNodeTransform *= (SMatrix(&node->mParent->mTransformation.a1).Transpose());
				
				if(it2 != _boneMap.end())
				{
					currentJoint.parent = &(it2->second);
					currentJoint.parent->offspring.push_back(&currentJoint);
				}
			}
		}

		for (unsigned int i = 0; i < node->mNumChildren; ++i)
			this->linkSkeletonTree(node->mChildren[i]);
	}



	bool processNode(ID3D11Device* dvc, aiNode* node, const aiScene* scene, float rUVx, float rUVy)	//aiMatrix4x4 parentTransform, 
	{
		//aiMatrix4x4 concatenatedTransform = parentTransform * node->mTransformation;	//or reversed! careful!
		
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
			meshes.push_back(processSkeletalMesh(dvc, scene->mMeshes[node->mMeshes[i]], scene, meshes.size(), rUVx, rUVy)); /*concatenatedTransform*/

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
		
		bool hasTexCoords = false;
		if (mesh->mTextureCoords[0])
			hasTexCoords = true;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++) 
		{
			BonedVert3D vertex;

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
		loadBones(*mesh, vertices);

		for (Texture& t : locTextures)
			t.Setup(device);

		return SkeletalMesh(vertices, indices, locTextures, device, ind);
	}



	std::vector<Texture> loadMaterialTextures(ID3D11Device* dvc, const aiScene* scene, aiMaterial *mat, aiTextureType type, std::string typeName)
	{
		std::vector<Texture> textures;

		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			boolean skip = false;

			mat->GetTexture(type, i, &str);

			// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture			
			for (unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				if (aiString(textures_loaded[j]._fileName) == str)
				{
					textures.push_back(textures_loaded[j]);
					skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}

			if (!skip)
			{   // If texture hasn't been loaded already, load it

				std::string fPath = directory + "/" + std::string(str.data);
				Texture texture(fPath);
				texture._typeName = typeName;

				//texture.Bind(type);
				bool loaded = texture.LoadFromStoredPath();

				if (!loaded)
				{
					//@TODO use the same version as in model... and stop duplicating code for all this anyways
					loaded = this->LoadEmbeddedTextures(dvc, textures, scene, fPath, type, typeName);	//for embedded textures

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



	bool LoadEmbeddedTextures(ID3D11Device* dvc, std::vector<Texture>& textures, const aiScene* scene, std::string& fPath, aiTextureType type, std::string& typeName)
	{
		if (scene->HasTextures())
		{
			for (size_t ti = 0; ti < scene->mNumTextures; ti++)
			{
				Texture texture(fPath);
				texture._typeName = typeName;

				size_t texSize = scene->mTextures[ti]->mWidth;

				//compressed textures tend to have height value 0
				if (scene->mTextures[ti]->mHeight != 0)
					texSize *= scene->mTextures[ti]->mHeight;

				texture.LoadFromMemory(reinterpret_cast<unsigned char*>(scene->mTextures[ti]->pcData), texSize);

				textures.push_back(texture);
				textures_loaded.push_back(texture);
			}

			return true;
		}
		return false;
	}



	void loadBones(const aiMesh& aiMesh, std::vector<BonedVert3D>& verts) 
	{
		if (!aiMesh.HasBones())
			return;

		int numBones = 0;

		for (unsigned int i = 0; i < aiMesh.mNumBones; ++i) 
		{
			aiBone* bone = aiMesh.mBones[i];
			int boneIndex;
			
			std::string boneName(bone->mName.data);

			//connect bone data and bone ids, if not found then add a new one, if found adjust its index
			if (_boneMap.find(boneName) == _boneMap.end())
			{
				boneIndex = numBones;

				SMatrix jointOffsetMat(&bone->mOffsetMatrix.a1);
				jointOffsetMat = jointOffsetMat.Transpose();
				
				Joint joint(boneIndex, boneName, jointOffsetMat);
				_boneMap.insert({ boneName, joint });
				numBones++;
			}
			else
			{
				boneIndex = _boneMap[boneName].index;	
			}


			//Load bone data (four bone indices, each weighted) into vertices, this works.
			for (unsigned int j = 0; j < bone->mNumWeights; ++j)
			{
				unsigned int vertID = bone->mWeights[j].mVertexId;
				float weight = bone->mWeights[j].mWeight;
				verts[vertID].AddBoneData(boneIndex, weight);
			}
		}
	}



	void loadAnimations(const aiScene* scene) 
	{
		if (!scene->HasAnimations())
			return;

		for (int i = 0; i < scene->mNumAnimations; ++i) 
		{
			auto sceneAnimation = scene->mAnimations[i];
			int numChannels = sceneAnimation->mNumChannels;

			Animation anim(std::string(sceneAnimation->mName.data), sceneAnimation->mDuration, sceneAnimation->mTicksPerSecond, numChannels);

			for (int j = 0; j < numChannels; ++j)
			{
				aiNodeAnim* channel = sceneAnimation->mChannels[j];

				//create empty channel object, for our use not like assimp's
				AnimChannel ac(channel->mNumPositionKeys, channel->mNumRotationKeys, channel->mNumScalingKeys);
				ac.jointName = std::string(channel->mNodeName.C_Str());

				for (int c = 0; c < channel->mNumScalingKeys; c++)
				{
					double time = channel->mScalingKeys[c].mTime;
					aiVector3D chScale = channel->mScalingKeys[c].mValue;
					SVec3 scale = SVec3(chScale.x, chScale.y, chScale.z);
					ac.sclVec.emplace_back(time, scale);
				}

				for (int b = 0; b < channel->mNumRotationKeys; b++) 
				{
					double time = channel->mRotationKeys[b].mTime;
					aiQuaternion chRot = channel->mRotationKeys[b].mValue;
					SQuat rot = SQuat(chRot.x, chRot.y, chRot.z, chRot.w);
					ac.rotVec.emplace_back(time, rot);
				}

				for (int a = 0; a < channel->mNumPositionKeys; a++)
				{
					double time = channel->mPositionKeys[a].mTime;
					aiVector3D chPos = channel->mPositionKeys[a].mValue;
					SVec3 pos = SVec3(chPos.x, chPos.y, chPos.z);
					ac.posVec.emplace_back(time, pos);
				}

				//add channel to animation
				anim.addChannel(ac);
			}

			//store animation for later use
			anims.push_back(anim);
		}
	}



	void MakeLikeATree()
	{
		for (auto njPair : _boneMap)
		{
			if (njPair.second.parent == nullptr)
				_rootJoint = njPair.second;
		}

		CalcGlobalTransforms(_rootJoint, SMatrix::Identity);
	}



	void CalcGlobalTransforms(Joint& j, const SMatrix& parentMat)
	{
		j.globalTransform = j.aiNodeTransform * parentMat;

		for (Joint* cj : j.offspring)
		{
			CalcGlobalTransforms(*cj, j.globalTransform);
		}
	}



	void update(float dTime, std::vector<SMatrix>& vec, UINT animIndex = 0u)
	{
		for (int i = 0; i < anims.size(); ++i)
			anims[i].update(dTime);

		getTransformAtTime(anims[animIndex], _rootJoint, vec, SMatrix::Identity);
	}



	void getTransformAtTime(const Animation& anim, Joint& joint, std::vector<SMatrix>& vec, const SMatrix& parentMatrix)
	{
		float currentTick = anim.getElapsed() / anim.getTickDuration();
		float t = fmod(anim.getElapsed(), anim.getTickDuration());

		SVec3 pos, scale;
		SQuat quat;

		AnimChannel channel;
		bool found = anim.getAnimChannel(joint.name, channel);
		
		//matrix that is created according to animation data and current time using interpolation, if no data use default
		SMatrix animationMatrix = joint.globalTransform;		

		//MATRIX j.aiNodeTransform IS NOT ABSOLUTE TO MESH ROOT, UNLIKE BONE OFFSETS!!! THESE MUST BE CHAINED INTO globalTransform

		if (found)
		{
			for (UINT i = 0; i < channel.posVec.size() - 1; ++i)
			{
				if (currentTick < (float)channel.posVec[i + 1].first)
				{
					SVec3 posPre = channel.posVec[i].second;
					SVec3 posPost = channel.posVec[i + 1 == channel.posVec.size() ? 0 : i + 1].second;
					pos = Math::lerp(posPre, posPost, t);
					break;
				}
			}

			for (UINT i = 0; i < channel.sclVec.size() - 1; ++i)
			{
				if (currentTick < (float)channel.sclVec[i + 1].first)
				{
					SVec3 scalePre = channel.sclVec[i].second;
					SVec3 scalePost = channel.sclVec[i + 1 == channel.sclVec.size() ? 0 : i + 1].second;
					scale = Math::lerp(scalePre, scalePost, t);
					break;
				}
			}

			for (UINT i = 0; i < channel.rotVec.size() - 1; ++i)
			{
				if (currentTick < (float)channel.rotVec[i + 1].first)
				{
					SQuat rotPre = channel.rotVec[i].second;
					SQuat rotPost = channel.rotVec[i + 1 == channel.rotVec.size() ? 0 : i + 1].second;
					quat = SQuat::Slerp(rotPre, rotPost, t);
					break;
				}
			}

			SMatrix sMat = SMatrix::CreateScale(scale);
			SMatrix rMat = SMatrix::CreateFromQuaternion(quat);
			SMatrix tMat = SMatrix::CreateTranslation(pos);

			animationMatrix = sMat * rMat * tMat;
		}

		SMatrix nodeTransform;
		
		if(found)
			nodeTransform = animationMatrix * joint.aiNodeTransform * parentMatrix;
		else
			nodeTransform = animationMatrix * parentMatrix;

		SMatrix finalMatrix = 
			joint.meshToLocalBoneSpaceTransform //go from mesh space to bone space
			* nodeTransform						//animate (or default bind if none), then go to parent space (as in, attach the bone to the parent
			* _globalInverseTransform			//move the entire mesh to origin
			;

		vec[joint.index] = finalMatrix.Transpose();	//transpose because the shader is column major, nothing to do with the animation process
		

		for (Joint* child : joint.offspring)
		{
			getTransformAtTime(anim, *child, vec, nodeTransform);
		}
	}



	void Draw(ID3D11DeviceContext* dc, ShaderSkelAnim& shader)
	{
		for (unsigned int i = 0; i < this->meshes.size(); i++)
			this->meshes[i].draw(dc, shader);
	}

private:

	inline bool fileExists(const std::string& name)
	{
		std::ifstream f(name.c_str());
		return f.good();
	}
};
