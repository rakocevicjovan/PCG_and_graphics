#pragma once
#include "assimp\Importer.hpp"	
#include "assimp\scene.h"
#include "assimp\postprocess.h" 
#include "Mesh.h"
#include "Animation.h"
#include "Texture.h"
#include "FileUtilities.h"
#include "Skeleton.h"



class AssimpWrapper
{
private:



public:

	static const aiScene* loadScene(Assimp::Importer& importer, const std::string& path, UINT pFlags)
	{
		assert(FileUtils::fileExists(path) && "File does not exist! ...probably.");

		const aiScene* scene = importer.ReadFile(path, pFlags);

		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::string errString("Assimp error:" + std::string(importer.GetErrorString()));
			OutputDebugStringA(errString.c_str());
			return nullptr;
		}

		return scene;
	}



	static void postProcess(Assimp::Importer& imp, aiPostProcessSteps steps)
	{
		imp.ApplyPostProcessing(steps);	//aiProcess_CalcTangentSpace
	}



	// returns bounding sphere radius
	template <typename VertexType>
	static float loadVertices(aiMesh* aiMesh, bool hasTexCoords, std::vector<VertexType>& verts)
	{
		verts.reserve(aiMesh->mNumVertices);

		UINT numTexChannels = aiMesh->GetNumUVChannels();	// Number of UV coordinate sets (channels)
		UINT* numUVComponents = aiMesh->mNumUVComponents;	// Whether the channel contains 1, 2 or 3 values (U, UV, UVW)
		
		float maxDist = 0.f;

		VertexType vertex;

		for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i)
		{
			//SVec3(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
			vertex.pos = SVec3(aiMesh->mVertices[i].x);	

			// Should be normalized already?
			vertex.normal = SVec3(&aiMesh->mNormals[i].x);

			vertex.texCoords = hasTexCoords ? SVec2(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y) : SVec2::Zero;

			verts.push_back(vertex);

			float curDist = vertex.pos.LengthSquared();
			if (maxDist < curDist) maxDist = curDist;
		}

		return maxDist;
	}



	static void loadIndices(aiMesh* aiMesh, std::vector<UINT>& indices)
	{
		aiFace face;
		for (UINT i = 0; i < aiMesh->mNumFaces; ++i)
		{
			face = aiMesh->mFaces[i];

			//populate indices from faces
			for (UINT j = 0; j < face.mNumIndices; ++j)
				indices.emplace_back(face.mIndices[j]);
		}
	}



	static void loadTangents(aiMesh* aiMesh, std::vector<Vert3D>& verts, std::vector<SVec3>& faceTangents)
	{
		aiFace face;
		for (UINT i = 0; i < aiMesh->mNumFaces; ++i)
		{
			faceTangents.emplace_back(calculateTangent(verts, face));	// Calculate tangents for faces
		}

		for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i)
		{
			face = aiMesh->mFaces[i];

			for (unsigned int j = 0; j < face.mNumIndices; ++j)			// Assign face tangents to vertex tangents
				verts[face.mIndices[j]].tangent += faceTangents[i];
		}

		for (Vert3D& vert : verts)
			vert.tangent.Normalize();
	}



	static void loadMaterials(const std::string& path, const aiScene* scene, aiMesh* aiMesh, std::vector<Texture>& textures)
	{
		if (aiMesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];

			// Diffuse maps
			loadMaterialTextures(path, textures, scene, material, aiTextureType_DIFFUSE, "texture_diffuse", DIFFUSE);

			//  Normal maps
			loadMaterialTextures(path, textures, scene, material, aiTextureType_NORMALS, "texture_normal", NORMAL);

			// Specular maps
			loadMaterialTextures(path, textures, scene, material, aiTextureType_SPECULAR, "texture_specular", SPECULAR);

			// Shininess maps
			loadMaterialTextures(path, textures, scene, material, aiTextureType_SHININESS, "texture_shininess", SHININESS);

			// Opacity maps
			loadMaterialTextures(path, textures, scene, material, aiTextureType_OPACITY, "texture_opacity", OPACITY);

			// Displacement maps
			loadMaterialTextures(path, textures, scene, material, aiTextureType_DISPLACEMENT, "texture_disp", DISPLACEMENT);

			// Ambient occlusion maps
			loadMaterialTextures(path, textures, scene, material, aiTextureType_AMBIENT, "texture_AO", AMBIENT);

			// Other maps
			loadMaterialTextures(path, textures, scene, material, aiTextureType_UNKNOWN, "texture_other", OTHER);

			// Weird properties... that I never really saw trigger
			loadMaterialTextures(path, textures, scene, material, aiTextureType_NONE, "texture_property", OTHER);
		}
	}



	static bool loadMaterialTextures(
		std::string modelPath,
		std::vector<Texture>& textures, 
		const aiScene* scene, 
		aiMaterial *mat,
		aiTextureType type, 
		std::string typeName,
		TextureRole role)
	{
		//iterate all textures of relevant related to the material
		for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
		{
			aiString obtainedTexturePath;
			mat->GetTexture(type, i, &obtainedTexturePath);

			// Assumes relative paths
			std::string texPath = modelPath.substr(0, modelPath.find_last_of("/\\")) + "/" + std::string(obtainedTexturePath.data);
			Texture curTexture;
			curTexture._fileName = texPath;
			curTexture._typeName = typeName;
			curTexture._role = role;

			// Try to load from file
			bool loaded = curTexture.LoadFromStoredPath();

			// Load from file failed - embedded (attempt to load from memory) or corrupted
			if (!loaded)
			{
				int embeddedIndex = atoi(obtainedTexturePath.C_Str() + sizeof(char));	//skip the * with + sizeof(char)
				loaded = loadEmbeddedTexture(curTexture, scene, embeddedIndex);
			}

			// Load failed completely - most likely the data is corrupted or my library doesn't support it
			if (!loaded)
			{
				OutputDebugStringA("TEX_LOAD::Texture did not load! \n"); //@TODO use logger here instead
				continue;
			}

			textures.push_back(curTexture);	// Should try to do std::move when this is done
		}
		return true;
	}



	static bool loadEmbeddedTexture(Texture& texture, const aiScene* scene, UINT index)
	{
		if (!scene->mTextures)
			return false;

		aiTexture* aiTex = scene->mTextures[index];

		if (!aiTex)
			return false;

		size_t texSize = aiTex->mWidth;

		//compressed textures could have height value of 0
		if (aiTex->mHeight != 0)
			texSize *= aiTex->mHeight;

		texture.LoadFromMemory(reinterpret_cast<unsigned char*>(aiTex->pcData), texSize);

		return true;
	}



	static void loadAnimations(const aiScene* scene, std::vector<Animation>& outAnims)
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
					ac.sKeys.emplace_back(channel->mScalingKeys[c].mTime, SVec3(&channel->mScalingKeys[c].mValue.x));
				}

				for (int b = 0; b < channel->mNumRotationKeys; b++)
				{
					ac.rKeys.emplace_back(channel->mRotationKeys[b].mTime, SQuat(&channel->mRotationKeys[b].mValue.x));
				}

				for (int a = 0; a < channel->mNumPositionKeys; a++)
				{
					ac.pKeys.emplace_back(channel->mPositionKeys[a].mTime, SVec3(&channel->mPositionKeys[a].mValue.x));
				}

				anim.addChannel(ac);
			}

			outAnims.push_back(anim);
		}
	}



	static void loadBones(const aiMesh& aiMesh, std::vector<BonedVert3D>& verts, Skeleton& skeleton)
	{
		if (!aiMesh.HasBones())
			return;

		int numBones = 0;

		for (unsigned int i = 0; i < aiMesh.mNumBones; ++i)
		{
			aiBone* bone = aiMesh.mBones[i];
			int boneIndex = 0;

			std::string boneName(bone->mName.data);

			// Connect bone indices to vertex skinning data

			int foundIndex = skeleton.getBoneIndex(boneName);	// Find a bone with a matching name in the skeleton

			if (foundIndex > 0)
			{
				boneIndex = foundIndex;	// Bone found, use it's index for skinning
			}
			else	// Bone doesn't exist in our skeleton data yet, add it, then use it's index for skinning
			{
				boneIndex = numBones;

				SMatrix jointOffsetMat(&bone->mOffsetMatrix.a1);
				jointOffsetMat = jointOffsetMat.Transpose();

				Joint joint(boneIndex, boneName, jointOffsetMat);
				skeleton._boneMap.insert({ boneName, joint });
				++numBones;
			}


			// Load skinning data (up to four bone indices and four weights) into vertices
			for (unsigned int j = 0; j < bone->mNumWeights; ++j)
			{
				unsigned int vertID = bone->mWeights[j].mVertexId;
				float weight = bone->mWeights[j].mWeight;
				verts[vertID].AddBoneData(boneIndex, weight);
			}
		}
	}





	// Helper stuff

	static SVec3 calculateTangent(const std::vector<Vert3D>& vertices, const aiFace& face)
	{
		if (face.mNumIndices < 3) return SVec3(0, 0, 0);

		SVec3 tangent;
		SVec3 edge1, edge2;
		SVec2 duv1, duv2;

		//Find first texture coordinate edge 2d vector
		Vert3D v0 = vertices[face.mIndices[0]];
		Vert3D v1 = vertices[face.mIndices[1]];
		Vert3D v2 = vertices[face.mIndices[2]];

		edge1 = v0.pos - v2.pos;
		edge2 = v2.pos - v1.pos;

		duv1 = v0.texCoords - v2.texCoords;
		duv2 = v2.texCoords - v1.texCoords;

		float f = 1.0f / (duv1.x * duv2.y - duv2.x * duv1.y);

		//Find tangent using both tex coord edges and position edges
		tangent.x = (duv1.y * edge1.x - duv2.y * edge2.x) * f;
		tangent.y = (duv1.y * edge1.y - duv2.y * edge2.y) * f;
		tangent.z = (duv1.y * edge1.z - duv2.y * edge2.z) * f;

		tangent.Normalize();

		return tangent;
	}

};