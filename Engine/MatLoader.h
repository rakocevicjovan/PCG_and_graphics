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



	static std::vector<Material*> LoadAllMaterials(
		ID3D11Device* device, const aiScene* scene, const std::string& modPath)
	{
		std::vector<Material*> materials(scene->mNumMaterials);

		std::map<std::string, std::shared_ptr<Texture>> texNamePtrMap;

		for (UINT i = 0; i < materials.size(); ++i)
			materials[i] = LoadMaterial(device, scene, scene->mMaterials[i], modPath, texNamePtrMap);

		return materials;
	}



	static Material* LoadMaterial(ID3D11Device* device, const aiScene* scene, const aiMaterial* aiMat,
		const std::string& modelPath, std::map<std::string, std::shared_ptr<Texture>>& texNamePtrMap)
	{
		Material* mat = new Material();
		
		// Parameters - once I decide what to support, parse and load into a cbuffer
		//loadParameterBlob(aiMat);
		
		// Textures
		std::vector<TempTexData> tempTexData;
		for (AssimpWrapper::TEX_TYPE_ROLE ttr : AssimpWrapper::ASSIMP_TEX_TYPES)
			GetTexMetaData(aiMat, ttr, tempTexData);
		
		mat->_texMetaData.resize(tempTexData.size());
		
		// Set pointers to textures...
		for (UINT i = 0; i < tempTexData.size(); ++i)
		{
			mat->_texMetaData[i] = tempTexData[i]._tmd;

			auto iter = texNamePtrMap.insert({ tempTexData[i]._path, nullptr });

			if (iter.second)	// Did not exist, load up into map
			{
				std::shared_ptr<Texture>& t = iter.first->second;
				t.reset(LoadTexture(scene, modelPath, tempTexData[i]._path));
				if(t)
					(t)->SetUpAsResource(device, false);
			}
			// Assign to the metadata whether it did or didn't exist
			mat->_texMetaData[i]._tex = iter.first->second;
		}
		
		return mat;
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



	static Texture* LoadTexture(const aiScene* scene, const std::string& modelPath, const std::string& texPath)
	{
		Texture* curTex = new Texture();
		const char* texName = aiScene::GetShortFilename(texPath.c_str());

		// Check if embedded first
		bool loaded = AssimpWrapper::loadEmbeddedTexture(*curTex, scene, texPath.c_str());
		curTex->_fileName = texName;

		// Not embedded, try to load from file
		if (!loaded)
		{
			// Assumes relative paths
			std::string modelFolderPath = modelPath.substr(0, modelPath.find_last_of("/\\"));
			std::string absTexPath = modelFolderPath + "/" + texPath;

			// Path is faulty, try to find it under model directory
			if (!std::filesystem::exists(absTexPath))
			{
				std::filesystem::directory_entry texFile;
				if (FileUtils::findFile(modelFolderPath, texName, texFile))
					absTexPath = texFile.path().string();
			}

			loaded = curTex->loadFromPath(absTexPath.c_str());
			curTex->_fileName = absTexPath;
		}

		// Load failed, likely the data is corrupted or stb doesn't support it
		if (!loaded)
		{
			delete curTex;
			curTex = nullptr;
			OutputDebugStringA("TEX_LOAD::Texture did not load! \n"); //@TODO use logger here instead
		}

		return curTex;
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



	// New version with multithreading, do eventually
	/*
	static std::vector<MatMetaData> LoadMaterialMetaData(const aiScene* scene)
	{
		std::vector<MatMetaData> result(scene->mNumMaterials);

		for (UINT i = 0; i < scene->mNumMaterials; ++i)
		{
			aiMaterial* aiMat = scene->mMaterials[i];

			for (AssimpWrapper::TEX_TYPE_ROLE ttr : AssimpWrapper::ASSIMP_TEX_TYPES)
			{
				GetTexMetaData(aiMat, ttr, result[i]._tempTexData);
			}
		}

		return result;
	}



	static void LoadAllTextures(std::vector<MatMetaData>& matMetaData)
	{
		std::set<std::string> unqTexNames;

		for (const auto& mmd : matMetaData)
			for (const auto& ttd : mmd._tempTexData)
				unqTexNames.insert(ttd._path);

		UINT nUnqTex = unqTexNames.size();
		UINT nThreads = nUnqTex - 1;

		ctpl::thread_pool threadPool(nThreads);	// Might be smarter to adjust the number somehow

		std::vector<std::future<void>> futures(nThreads);

		UINT counter = 0u;
		for (const auto& path : unqTexNames)
		{
			// Load multithreaded
			
			futures[counter] = threadPool.push
			(std::bind(
				LoadTexture()	// tbd
			));
			++counter;
		}
	}
	*/
};