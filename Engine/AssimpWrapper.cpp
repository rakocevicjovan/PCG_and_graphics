#include "AssimpWrapper.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "VertSignature.h"
#include "SkeletalModel.h"
#include <array>



const aiScene* AssimpWrapper::loadScene(Assimp::Importer& importer, const std::string& path, UINT pFlags)
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



std::vector<Material*> AssimpWrapper::loadMaterials(aiScene* scene, const std::string& path)
{
	std::vector<Material*> materials;
	std::vector<Texture> textures;

	for (int i = 0; i < scene->mNumMaterials; ++i)
	{
		materials.push_back(new Material());
		loadMaterial(scene, i, path, materials.back(), textures);
	}

	return materials;
}



void AssimpWrapper::loadMaterial(const aiScene* scene, UINT index, const std::string& path, Material* mat, std::vector<Texture>& textures)
{
	if (index >= 0 && index < scene->mNumMaterials)
	{
		aiMaterial* aiMat = scene->mMaterials[index];

		// Textures
		// Diffuse maps
		loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_DIFFUSE, DIFFUSE);

		//  Normal maps
		loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_NORMALS, NORMAL);

		// Specular maps
		loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_SPECULAR, SPECULAR);

		// Shininess maps
		loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_SHININESS, SHININESS);

		// Opacity maps
		loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_OPACITY, OPACITY);

		// Displacement maps
		loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_DISPLACEMENT, DISPLACEMENT);

		// Ambient occlusion maps
		loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_AMBIENT, AMBIENT);

		// Metallic maps
		loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_METALNESS, METALLIC);

		// Other maps
		loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_UNKNOWN, OTHER);
	}
}



bool AssimpWrapper::loadMaterialTextures(
	const std::string& modelPath,
	std::vector<Texture>& textures,
	const aiScene* scene,
	aiMaterial *aiMat,
	Material* mat,
	aiTextureType aiTexType,
	TextureRole role)
{
	static const std::map<aiTextureMapMode, TextureMapMode> TEXMAPMODE_MAP
	{
		{aiTextureMapMode_Wrap,		TextureMapMode::WRAP},
		{aiTextureMapMode_Clamp,	TextureMapMode::CLAMP},
		{aiTextureMapMode_Decal,	TextureMapMode::BORDER},
		{aiTextureMapMode_Mirror,	TextureMapMode::MIRROR}
	};

	// Iterate all textures related to the material, keep the ones that can load
	for (UINT i = 0; i < aiMat->GetTextureCount(aiTexType); ++i)
	{
		Texture curTexture;

		aiString aiTexPath;
		UINT uvIndex = 0u;
		aiTextureMapMode aiMapModes[3] {aiTextureMapMode_Wrap, aiTextureMapMode_Wrap , aiTextureMapMode_Wrap };

		aiMat->GetTexture(aiTexType, i, &aiTexPath, nullptr, &uvIndex, nullptr, nullptr, &aiMapModes[0]);
		
		TextureMapMode mapModes[3];
		for (UINT j = 0; j < 3; ++j)
			mapModes[j] = TEXMAPMODE_MAP.at(aiMapModes[j]);

		std::string texName(aiScene::GetShortFilename(aiTexPath.C_Str()));

		// Check if embedded first, not the most common case but faster to check anyways
		bool loaded = loadEmbeddedTexture(curTexture, scene, &aiTexPath);
		curTexture._fileName = texName;

		// Not embedded, try to load from file
		if (!loaded)
		{
			// Assumes relative paths
			std::string modelFolderPath = modelPath.substr(0, modelPath.find_last_of("/\\"));
			std::string texPath = modelFolderPath + "/" + std::string(aiTexPath.data);

			// If path is faulty, try to find it in the model directory and subdirectories
			if (!std::filesystem::exists(texPath))
			{
				std::filesystem::directory_entry texFile;
				if (FileUtils::findFile(modelFolderPath, texName, texFile))
					texPath = texFile.path().string();
			}

			curTexture._fileName = texPath;	// or texName, I really don't even know why either tbh

			loaded = curTexture.LoadFromStoredPath();
		}

		// Load failed completely - most likely the data is corrupted or my library doesn't support it
		if (!loaded)
		{
			OutputDebugStringA("TEX_LOAD::Texture did not load! \n"); //@TODO use logger here instead
			continue;
		}

		textures.push_back(curTexture);	// Should try to do std::move when this is done
		
		mat->_texMetaData.push_back({ 
			role, 
			reinterpret_cast<Texture*>(textures.size() - 1),
			{mapModes[0], mapModes[1], mapModes[2]},
			static_cast<uint8_t>(uvIndex)
			});	// Textures will relocate
	}
	return true;
}



bool AssimpWrapper::loadEmbeddedTexture(Texture& texture, const aiScene* scene, aiString* str)
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



void AssimpWrapper::loadAnimations(const aiScene* scene, std::vector<Animation>& outAnims)
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



void AssimpWrapper::loadBonesAndSkinData(const aiMesh& aiMesh, std::vector<BonedVert3D>& verts, Skeleton& skeleton)
{
	if (!aiMesh.HasBones())
		return;

	for (UINT i = 0; i < aiMesh.mNumBones; ++i)
	{
		aiBone* aiBone = aiMesh.mBones[i];

		std::string boneName(aiBone->mName.data);

		// Find a bone with a matching name in the skeleton
		int boneIndex = skeleton.getBoneIndex(boneName);

		if (boneIndex < 0)	// Bone doesn't exist in our skeleton data yet, add it
		{
			boneIndex = skeleton._bones.size();//getBoneCount();

			SMatrix boneOffsetMat = aiMatToSMat(aiBone->mOffsetMatrix);

			skeleton._bones.push_back(Bone(boneIndex, boneName, boneOffsetMat));
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



void AssimpWrapper::addMissingBones(Skeleton* skeleton, const aiNode* boneNode, SMatrix meshGlobalMatrix)
{
	/*aiNode* parent = boneNode->mParent;

	if (!parent)			// We are at root node, no way but down (also prevents crashing below)
		return;

	if (!parent->mParent)	// Don't include the root node either... bit hacky but works out so far
		return;

	std::string parentName(parent->mName.C_Str());

	if (skeleton->boneExists(parentName))	// Parent is already a bone, terminate
		return;

	Bone newParentBone;
	newParentBone._name = parentName;
	newParentBone._index = skeleton->getBoneCount();
	skeleton->insertBone(newParentBone);

	addMissingBones(skeleton, parent, meshGlobalMatrix);
	*/
}



const aiNode* AssimpWrapper::findSkeletonRoot(const aiNode* node, Skeleton& skeleton, SMatrix pMat)
{
	const aiNode* result = nullptr;

	// Make skeleton root account for all nodes before it 
	// Usually it's a direct child of root but not always
	SMatrix nodeLocalTransform = aiMatToSMat(node->mTransformation);
	pMat = nodeLocalTransform * pMat;

	Bone* bone = &skeleton._bones[0];	//skeleton.findBone(node->mName.C_Str());

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



void AssimpWrapper::loadAllBoneNames(const aiScene* scene, aiNode* node, std::set<std::string>& boneNames)
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



// Helpers
std::vector<aiString> AssimpWrapper::getExtTextureNames(const aiScene* scene)
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



SVec3 AssimpWrapper::calcFaceTangent(const std::vector<Vert3D>& vertices, const aiFace& face)
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



aiNode* AssimpWrapper::findModelNode(aiNode* node, SMatrix& meshRootTransform)
{
	SMatrix locTrfm = aiMatToSMat(node->mTransformation);
	meshRootTransform = locTrfm * meshRootTransform;

	if (node->mNumMeshes > 0)
		return node;

	for (int i = 0; i < node->mNumChildren; ++i)
	{
		findModelNode(node->mChildren[i], meshRootTransform);
	}
}



SMatrix AssimpWrapper::getNodeGlobalTransform(const aiNode* node)
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



bool AssimpWrapper::containsRiggedMeshes(const aiScene* scene)
{
	for (int i = 0; i < scene->mNumMeshes; ++i)
		if (scene->mMeshes[i]->HasBones())
			return true;

	return false;
}



UINT AssimpWrapper::countChildren(const aiNode* node)
{
	UINT numChildren = node->mNumChildren;

	UINT result = numChildren;

	for (UINT i = 0; i < numChildren; ++i)
		result += countChildren(node->mChildren[i]);

	return result;
}