#pragma once
#include "Material.h"
#include "AssimpWrapper.h"
#include "TextureCache.h"
#include "Fnv1Hash.h"
#include <memory>



class MatLoader
{
private:

	struct TempTexData
	{
		std::string _path;
		TextureMetaData _tmd;
	};

public:

	static std::vector<Material> LoadAllMaterials(
		const aiScene* scene, const std::string& modPath, TextureCache* ptc)
	{
		std::vector<Material> materials;
		materials.reserve(scene->mNumMaterials);

		std::map<std::string, Texture*> texNamePtrMap;

		for (UINT i = 0; i < scene->mNumMaterials; ++i)
			materials.emplace_back(LoadMaterial(scene, scene->mMaterials[i], modPath, texNamePtrMap));

		return materials;
	}



	static Material LoadMaterial(const aiScene* scene, const aiMaterial* aiMat, 
		const std::string& modelPath, std::map<std::string, Texture*>& texNamePtrMap)
	{
		Material mat;
		
		// Parameters - once I decide what to support, parse and load into a cbuffer
		//loadParameterBlob(aiMat);
		
		// Textures
		std::vector<TempTexData> tempTexData;
		for (AssimpWrapper::TEX_TYPE_ROLE ttr : AssimpWrapper::ASSIMP_TEX_TYPES)
			GetTexMetaData(aiMat, ttr, tempTexData);
		
		mat._texMetaData.resize(tempTexData.size());
		
		// Set pointers to textures...
		for (UINT i = 0; i < tempTexData.size(); ++i)
		{
			mat._texMetaData[i] = tempTexData[i]._tmd;

			auto iter = texNamePtrMap.insert({ tempTexData[i]._path, nullptr });

			if (iter.second)	// Did not exist, load up
				iter.first->second = LoadTexture(scene, modelPath, tempTexData[i]._path);
			else				// Did exist, just use the existing pointer
				mat._texMetaData[i]._tex = texNamePtrMap[tempTexData[i]._path];
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

			//reinterpret_cast<Texture*>(fnv1hash(aiTexPath.C_Str())

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
		curTex->_fileName = texName;

		// Check if embedded first
		bool loaded = AssimpWrapper::loadEmbeddedTexture(*curTex, scene, texPath.c_str());

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
};