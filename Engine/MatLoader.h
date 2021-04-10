#pragma once
#include "Material.h"
#include "AssimpWrapper.h"
#include "TextureCache.h"
#include "Fnv1Hash.h"
#include "VitThreadPool.h"
#include <memory>



class MatLoader
{
public:

	struct TempTexData
	{
		std::string _path;
		TextureMetaData _tmd;
	};

	struct MatMetaData
	{
		// ParameterBundle _paramBundle;	// When decided upon.
		std::vector<TempTexData> _tempTexData;
	};

	struct TexNameBlob
	{ 
		std::string name; 
		Blob blob;
		bool embedded = false;	// Release instead of deleting, or double delete wrecks everything
	};

	struct MatsAndTextureBlobs
	{
		std::vector<std::shared_ptr<Material>> _mats;
		std::vector<TexNameBlob> _blobs;
	};


	static MatsAndTextureBlobs LoadAllMaterials(ID3D11Device* device, const aiScene* scene, const std::string& modPath)
	{
		uint32_t materialCount = scene->mNumMaterials;

		// Get material meta data, but defer loading materials, filter texture paths into a unique set
		std::vector<MatMetaData> matMetaData(materialCount);
		std::map<std::string, std::shared_ptr<Texture>> unqTexPaths;

		// Get material metadata, including textures that need to be loaded by path
		for (UINT i = 0; i < materialCount; ++i)
		{
			matMetaData[i] = LoadMatMetaData(scene->mMaterials[i]);
		}

		// Keep only unique texture names
		for (const auto& mmd : matMetaData)
		{
			for (auto& ttd : mmd._tempTexData)
				unqTexPaths.insert({ ttd._path, std::make_shared<Texture>() });
		}

		// Load uniquely identified textures as data blobs (in order to persist them later)
		std::vector<TexNameBlob> texNameBlobs;

		for (auto& pathTex : unqTexPaths)
		{
			TexNameBlob tnb = MatLoader::GetTextureBlob(scene, modPath, pathTex.first);
			
			if (tnb.blob._size == 0)	// Skip textures that were not found, for now.
				continue;

			pathTex.second.get()->loadFromMemory(reinterpret_cast<unsigned char*>(tnb.blob._data.get()), tnb.blob._size);
			pathTex.second.get()->setUpAsResource(device);

			texNameBlobs.push_back(std::move(tnb));
		}
		
		// Create materials from provided metadata and assign already loaded textures to them
		std::vector<std::shared_ptr<Material>> materials;
		materials.reserve(materialCount);

		for (const MatMetaData& mmd : matMetaData)
		{
			std::shared_ptr<Material> curMat = std::make_shared<Material>();
			
			UINT numTexRefs = mmd._tempTexData.size();
			curMat->_texMetaData.reserve(numTexRefs);

			for (const TempTexData& tempTexData : mmd._tempTexData)
			{
				curMat->_texMetaData.push_back(tempTexData._tmd);
				curMat->_texMetaData.back()._tex = unqTexPaths[tempTexData._path];
			}

			materials.push_back(std::move(curMat));
		}

		return { materials, std::move(texNameBlobs) };
	}


private:

	static MatMetaData LoadMatMetaData(const aiMaterial* aiMat)
	{
		//loadParameterBlob(aiMat);	// @TODO eventually

		// Textures
		std::vector<TempTexData> tempTexData;
		for (AssimpWrapper::TEX_TYPE_ROLE ttr : AssimpWrapper::ASSIMP_TEX_TYPES)
			GetTexMetaData(aiMat, ttr, tempTexData);

		return { tempTexData };
	}


	static void GetTexMetaData(const aiMaterial *aiMat, AssimpWrapper::TEX_TYPE_ROLE ttr, std::vector<TempTexData>& ttd)
	{
		// Iterate all textures related to the material, keep the ones that can load
		UINT texCountByType = aiMat->GetTextureCount(ttr.first);
		for (UINT i = 0; i < texCountByType; ++i)
		{
			aiString aiTexPath;
			UINT uvIndex = 0u;
			aiTextureMapMode aiMapModes[3]{ aiTextureMapMode_Wrap, aiTextureMapMode_Wrap, aiTextureMapMode_Wrap };

			aiMat->GetTexture(ttr.first, i, &aiTexPath, nullptr, &uvIndex, nullptr, nullptr, &aiMapModes[0]);

			TextureMapMode mapModes[3];
			for (UINT j = 0; j < 3; ++j)
				mapModes[j] = AssimpWrapper::TEXMAPMODE_MAP.at(aiMapModes[j]);

			ttd.push_back(
				{
					aiTexPath.C_Str(),
					{
						nullptr,
						ttr.second,
						{ mapModes[0], mapModes[1], mapModes[2] },
						static_cast<uint8_t>(uvIndex),
						0u
					}
				});
		}
	}


	static TexNameBlob GetTextureBlob(const aiScene* scene, const std::string& modelPath, const std::string& texPath)
	{
		const char* texName = aiScene::GetShortFilename(texPath.c_str());

		const aiTexture* aiTex = scene->GetEmbeddedTexture(texPath.c_str());

		bool embedded = (aiTex != nullptr);

		if (embedded)
		{
			UINT len = aiTex->mHeight == 0 ? aiTex->mWidth : aiTex->mHeight * aiTex->mWidth;
			return { texPath, std::unique_ptr<char[]> (reinterpret_cast<char*>(aiTex->pcData)), len, true };
		}

		// Path is faulty, try to find it under model directory
		std::string modelFolderPath = std::filesystem::path(modelPath).parent_path().string();
		std::string absTexPath = modelFolderPath + "/" + texPath;

		if (!std::filesystem::exists(absTexPath))
		{
			std::filesystem::directory_entry texFile;
			if (FileUtils::findFile(modelFolderPath, texName, texFile))
				absTexPath = texFile.path().string();
			else
				return { texPath, Blob{} };
		}

		return { texPath, FileUtils::readAllBytes(absTexPath.c_str()) };
	}


	static void loadParameterBlob(aiMaterial* aiMat)
	{
		// Limit to those I want to support, make them shader key options (probs along with textures)
		// Also return a blob or stick it into Material*, currently does nothing

		aiString matName;
		if (aiMat->Get(AI_MATKEY_NAME, matName) != aiReturn_SUCCESS)
			matName = "not found";

		bool twoSided;
		if (aiMat->Get(AI_MATKEY_TWOSIDED, twoSided) != aiReturn_SUCCESS)
			twoSided = false;

		aiTextureOp blendFunc;
		if (aiMat->Get(AI_MATKEY_BLEND_FUNC, blendFunc) != aiReturn_SUCCESS)
			blendFunc = aiTextureOp_Multiply;

		float opacity;
		if (aiMat->Get(AI_MATKEY_OPACITY, opacity) != aiReturn_SUCCESS)
			opacity = 1.f;

		float transparencyFactor;
		if (aiMat->Get(AI_MATKEY_TRANSPARENCYFACTOR, transparencyFactor) != aiReturn_SUCCESS)
			transparencyFactor = 0.f;

		float shininess;	// Uh which is which...
		if (aiMat->Get(AI_MATKEY_SHININESS, shininess) != aiReturn_SUCCESS)
			shininess = 0.f;

		float specularIntensity;
		if (aiMat->Get(AI_MATKEY_SHININESS_STRENGTH, specularIntensity) != aiReturn_SUCCESS)
			specularIntensity = 0.f;

		float reflectivity;
		if (aiMat->Get(AI_MATKEY_REFLECTIVITY, reflectivity) != aiReturn_SUCCESS)
			reflectivity = 0.f;

		float refractionIndex;	// I guess, who the f uses REFRACTI
		if (aiMat->Get(AI_MATKEY_REFRACTI, refractionIndex) != aiReturn_SUCCESS)
			refractionIndex = 1.f;

		aiColor4D diffuseColour;
		if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColour) != aiReturn_SUCCESS)
			diffuseColour = aiColor4D(.5f);

		aiColor4D ambientColour;
		if (aiMat->Get(AI_MATKEY_COLOR_AMBIENT, ambientColour) != aiReturn_SUCCESS)
			ambientColour = aiColor4D(.5);

		aiColor4D specColour;
		if (aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specColour) != aiReturn_SUCCESS)
			specColour = aiColor4D(.5, .5, .5, 1.);

		aiColor4D emissiveColour;
		if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColour) != aiReturn_SUCCESS)
			emissiveColour = aiColor4D(0., 0., 0., 1.);

		aiColor4D transColour;
		if (aiMat->Get(AI_MATKEY_COLOR_TRANSPARENT, transColour) != aiReturn_SUCCESS)
			transColour = aiColor4D(0.f);

		aiColor4D reflectiveColour;
		if (aiMat->Get(AI_MATKEY_COLOR_REFLECTIVE, reflectiveColour) != aiReturn_SUCCESS)
			reflectiveColour = aiColor4D(0., 0., 0., 1.);

	}
};