#pragma once
#include "FileUtilities.h"

#include "Mesh.h"
#include "Texture.h"
#include "Skeleton.h"
#include "Animation.h"

#include <set>


class AssimpWrapper
{
public:

	static const aiScene* loadScene(Assimp::Importer& importer, const std::string& path, UINT pFlags)
	{
		assert(FileUtils::fileExists(path) && "File does not exist! ...probably.");

		const aiScene* scene = importer.ReadFile(path, pFlags);

		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::string errString("Assimp error: " + std::string(importer.GetErrorString()));
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
	template <typename VertexType> static float loadVertices(aiMesh* aiMesh, bool hasTexCoords, std::vector<VertexType>& verts)
	{
		verts.reserve(aiMesh->mNumVertices);

		float maxDist = 0.f;
		VertexType vertex;

		for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i)
		{
			vertex.pos = SVec3(&aiMesh->mVertices[i].x);

			vertex.texCoords = hasTexCoords ? SVec2(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y) : SVec2::Zero;

			vertex.normal = SVec3(&aiMesh->mNormals[i].x);	// Should be normalized already?

			verts.push_back(vertex);

			float curDist = vertex.pos.LengthSquared();
			if (maxDist < curDist) maxDist = curDist;
		}

		return sqrt(maxDist);
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


	// Template here as well? I need to think well about different vertex types
	static void loadTangents(aiMesh* aiMesh, std::vector<Vert3D>& verts, std::vector<SVec3>& faceTangents)
	{
		aiFace face;
		for (UINT i = 0; i < aiMesh->mNumFaces; ++i)
		{
			face = aiMesh->mFaces[i];
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



	static void loadMeshMaterial(const std::string& path, const aiScene* scene, aiMesh* aiMesh, std::vector<Texture>& textures)
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
			curTexture._role = role;

			// Try to load from file
			bool loaded = curTexture.LoadFromStoredPath();

			// Load from file failed - embedded (attempt to load from memory) or corrupted
			if (!loaded)
			{
				loaded = loadEmbeddedTexture(curTexture, scene, &obtainedTexturePath);
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



	static bool loadEmbeddedTexture(Texture& texture, const aiScene* scene, aiString* str)
	{
		const aiTexture* aiTex = scene->GetEmbeddedTexture(str->C_Str());

		if (!aiTex)
			return false;

		UINT texSize = aiTex->mWidth;

		if (aiTex->mHeight != 0)	//compressed textures could have height value of 0
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

				AnimChannel ac(channel->mNumPositionKeys, channel->mNumRotationKeys, channel->mNumScalingKeys);
				ac._boneName = std::string(channel->mNodeName.C_Str());

				for (int c = 0; c < channel->mNumScalingKeys; c++)
					ac._sKeys.emplace_back(channel->mScalingKeys[c].mTime, SVec3(&channel->mScalingKeys[c].mValue.x));

				for (int b = 0; b < channel->mNumRotationKeys; b++)
					ac._rKeys.emplace_back(channel->mRotationKeys[b].mTime, aiQuatToSQuat(channel->mRotationKeys[b].mValue));

				for (int a = 0; a < channel->mNumPositionKeys; a++)
					ac._pKeys.emplace_back(channel->mPositionKeys[a].mTime, SVec3(&channel->mPositionKeys[a].mValue.x));

				anim.addChannel(ac);
			}

			outAnims.push_back(anim);
		}
	}



	static void loadBonesAndSkinData(const aiMesh& aiMesh, std::vector<BonedVert3D>& verts, Skeleton& skeleton, SMatrix global)
	{
		if (!aiMesh.HasBones())
			return;

		for (UINT i = 0; i < aiMesh.mNumBones; ++i)
		{
			aiBone* aiBone = aiMesh.mBones[i];

			std::string boneName(aiBone->mName.data);

			// Connect bone indices to vertex skinning data
			int boneIndex = skeleton.getBoneIndex(boneName);	// Find a bone with a matching name in the skeleton

			if (boneIndex < 0)	// Bone doesn't exist in our skeleton data yet, add it, then use its index for skinning		
			{
				boneIndex = skeleton._boneMap.size();

				SMatrix boneOffsetMat = aiMatToSMat(aiBone->mOffsetMatrix);

				Bone bone(boneIndex, boneName, boneOffsetMat);

				skeleton._boneMap.insert({ boneName, bone });
			}


			// Load skinning data (up to four bone indices and four weights) into vertices
			for (UINT j = 0; j < aiBone->mNumWeights; ++j)
			{
				UINT vertID = aiBone->mWeights[j].mVertexId;
				float weight = aiBone->mWeights[j].mWeight;
				verts[vertID].AddBoneData(boneIndex, weight);
			}
		}
	}


	// Loads bones with direct influnce on vertices
	static void loadBones(const aiScene* scene, const aiNode* node, Skeleton& skeleton)
	{
		for (int i = 0; i < node->mNumMeshes; ++i)	// Iterate through meshes in a node
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			for (UINT j = 0; j < mesh->mNumBones; ++j)	// For each bone referenced by the mesh
			{
				aiBone* bone = mesh->mBones[j];

				std::string boneName(bone->mName.data);

				int boneIndex = skeleton.getBoneIndex(boneName);	// Check if this bone is already added

				if (boneIndex < 0)	// Bone wasn't added already, add it now
				{
					boneIndex = skeleton._boneMap.size();
					
					SMatrix boneOffsetMat = aiMatToSMat(bone->mOffsetMatrix);

					skeleton._boneMap.insert({ boneName, Bone(boneIndex, boneName, boneOffsetMat) });
				}
			}
		}

		for (UINT i = 0; i < node->mNumChildren; ++i)	// Repeat recursively
		{
			loadBones(scene, node->mChildren[i], skeleton);
		}
	}



	// Seeks upwards from every existing bone, filling in intermediate nodes
	static void addMissingBones(Skeleton& skeleton, const aiNode* boneNode, SMatrix meshGlobalMatrix)
	{
		aiNode* parent = boneNode->mParent;

		if (!parent)			// We are at root node, no way but down
			return;

		if (!parent->mParent)	// Don't include the root node either... bit hacky but works out so far
			return;

		std::string parentName(parent->mName.C_Str());

		if (skeleton.boneExists(parentName))	// Parent is already a bone, terminate
			return;

		Bone newParentBone;
		newParentBone.name = parentName;
		newParentBone.index = skeleton._boneMap.size();
		//newParentBone._offsetMatrix = AssimpWrapper::calculateOffsetMatrix(boneNode, meshGlobalMatrix);
		auto boneIter = skeleton._boneMap.insert({ parentName, newParentBone });

		addMissingBones(skeleton, parent, meshGlobalMatrix);
	}



	static const aiNode* findSkeletonRoot(const aiNode* node, Skeleton& skeleton, SMatrix pMat)
	{
		const aiNode* result = nullptr;

		// Make skeleton root account for all nodes before it 
		// Usually it's a direct child of root but not always
		SMatrix nodeLocalTransform = aiMatToSMat(node->mTransformation);
		pMat = nodeLocalTransform * pMat;

		Bone* bone = skeleton.findBone(node->mName.C_Str());

		if (bone)
		{
			bone->_localMatrix = pMat;	//pMat;
			result = node;
		}
		else
		{
			for (int i = 0; i < node->mNumChildren; ++i)
			{
				result = findSkeletonRoot(node->mChildren[i], skeleton, pMat);

				if (result)
					break;
			}
		}

		return result;
	}



	static void findModelNode(const aiNode* node, SMatrix& meshRootTransform)
	{
		SMatrix locTrfm = aiMatToSMat(node->mTransformation);
		meshRootTransform = locTrfm * meshRootTransform;

		if (node->mNumMeshes > 0)
			return;

		for (int i = 0; i < node->mNumChildren; ++i)
		{
			findModelNode(node->mChildren[i], meshRootTransform);
		}
	}



	static void loadAllBoneNames(const aiScene* scene, aiNode* node, std::set<std::string>& boneNames)
	{
		for (UINT i = 0; i < node->mNumMeshes; ++i)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			for (UINT i = 0; i < mesh->mNumBones; ++i)
			{
				aiBone* bone = mesh->mBones[i];
				boneNames.insert(std::string(bone->mName.C_Str()));
			}
		}

		for (UINT i = 0; i < node->mNumChildren; ++i)
		{
			loadAllBoneNames(scene, node->mChildren[i], boneNames);
		}
	}



	static bool containsRiggedMeshes(const aiScene* scene)
	{
		for (int i = 0; i < scene->mNumMeshes; ++i)
			if (scene->mMeshes[i]->HasBones())
				return true;

		return false;
	}



	static bool isOnlySkeleton(const aiScene* scene)
	{
		return (scene->mNumMeshes == 0);
	}



	static void loadOnlySkeleton(aiNode* node, Skeleton& skeleton, SMatrix parent)
	{
		SMatrix locTf = aiMatToSMat(node->mTransformation);
		parent = locTf * parent;

		Bone bone;
		bone.name = std::string(node->mName.C_Str());
		bone.index = skeleton._boneMap.size();
		bone._localMatrix = locTf;
		// Does not account for mesh offset, that has to be added when attaching mesh to skeleton
		bone._offsetMatrix = parent.Invert();	

		if (node->mParent)
		{
			std::string parentName(node->mParent->mName.C_Str());
			bone.parent = skeleton.findBone(parentName);
		}
	
		auto iter = skeleton._boneMap.insert({ bone.name, bone });

		if(bone.parent)	// Add links between parents and children, avoid crashing on root
			bone.parent->offspring.push_back(&iter.first->second);	// Looks awful but ayy... faster than searching

		for (int i = 0; i < node->mNumChildren; ++i)
			loadOnlySkeleton(node->mChildren[i], skeleton, parent);
	}



	static std::vector<aiString> getExtTextureNames(const aiScene* scene)
	{
		std::vector<aiString> result;

		for (UINT i = 0; i < scene->mNumMaterials; ++i)
		{
			aiMaterial* mat = scene->mMaterials[i];
			result.push_back(aiString(std::string("Mat: ") + mat->GetName().C_Str()));

			for (int j = aiTextureType::aiTextureType_NONE; j <= aiTextureType_UNKNOWN; ++j)
			{
				aiTextureType curType = static_cast<aiTextureType>(j);
				UINT curCount = mat->GetTextureCount(curType);

				for (UINT k = 0; k < curCount; ++k)
				{
					aiString texPath;
					mat->GetTexture(curType, k, &texPath);

					const aiTexture* aiTex = scene->GetEmbeddedTexture(texPath.C_Str());

					if (!aiTex)	// Only interested in external for this function
						result.push_back(texPath);
				}
			}
		}
		return result;
	}


	// Helpers

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



	inline static SMatrix getNodeGlobalTransform(const aiNode* node)
	{
		const aiNode* current = node;
		SMatrix concat = SMatrix::Identity;		// c * p * pp * ppp * pppp...

		while (current)
		{
			SMatrix localTransform = aiMatToSMat(current->mTransformation);
			concat *= localTransform;
			current = current->mParent;
		}

		return concat;
	}



	inline static SMatrix calculateOffsetMatrix(const aiNode* boneNode, SMatrix meshGlobalMat)
	{
		SMatrix boneNodeGlobalTransform = getNodeGlobalTransform(boneNode);

		SMatrix myOffsetMatrix = meshGlobalMat * boneNodeGlobalTransform.Invert();

		return myOffsetMatrix;
	}




	inline static SMatrix aiMatToSMat(const aiMatrix4x4& aiMat) { return SMatrix(&aiMat.a1).Transpose(); }



	inline static SQuat aiQuatToSQuat(const aiQuaternion& aq) { return SQuat(aq.x, aq.y, aq.z, aq.w); }
};