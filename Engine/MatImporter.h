#pragma once
#include "Material.h"
#include "AssimpWrapper.h"
#include "TextureCache.h"
#include "VitThreadPool.h"
#include <memory>
#include <execution>
#include <mutex>


class MatImporter
{
public:

	struct TexturePathAndMetadata
	{
		std::string path;
		TextureMetaData metaData;
	};

	struct MatMetaData
	{
		std::vector<TexturePathAndMetadata> _tempTexData;
		// ParameterBundle _paramBundle;	// When decided upon.
	};

	struct RawTextureData
	{
		std::string name;
		Blob blob;
		bool embedded = false;	// Assimp will release embedded textures
	};

	struct MatsAndTextureBlobs
	{
		std::vector<MatMetaData> _matMetaData;
		std::vector<std::shared_ptr<Material>> _mats;
		std::vector<RawTextureData> _blobs;
	};


	static MatsAndTextureBlobs ImportSceneMaterials(ID3D11Device* device, const aiScene* scene, const std::string& modPath)
	{
		const uint32_t materialCount{ scene->mNumMaterials };

		std::vector<MatMetaData> matMetaData(materialCount);

		// Get material metadata, including textures paths
		for (UINT i = 0; i < materialCount; ++i)
		{
			matMetaData[i] = LoadMatMetaData(scene->mMaterials[i]);
		}

		// Keep only unique textures, filtered by paths
		std::map<std::string, std::shared_ptr<Texture>> uniqueTextures;
		for (const auto& mmd : matMetaData)
		{
			for (auto& ttd : mmd._tempTexData)
			{
				uniqueTextures.insert({ ttd.path, std::make_shared<Texture>() });
			}
		}

		// Load uniquely identified textures as data blobs (in order to persist them later)
		auto texNameBlobs = LoadRawTextureData(scene, modPath, device, uniqueTextures);


		// Create materials from provided metadata and assign already loaded textures to them
		std::vector<std::shared_ptr<Material>> materials;
		materials.reserve(materialCount);

		for (const MatMetaData& mmd : matMetaData)
		{
			std::shared_ptr<Material> curMat = std::make_shared<Material>();

			UINT numTexRefs = mmd._tempTexData.size();
			curMat->_materialTextures.reserve(numTexRefs);

			for (const TexturePathAndMetadata& tempTexData : mmd._tempTexData)
			{
				curMat->_materialTextures.push_back({tempTexData.metaData, uniqueTextures[tempTexData.path]});
			}

			materials.push_back(std::move(curMat));
		}
		

		return { matMetaData, materials, std::move(texNameBlobs) };
	}


private:

	static MatMetaData LoadMatMetaData(const aiMaterial* aiMat)
	{
		// @TODO eventually
		//loadParameterBlob(aiMat);

		// Textures
		std::vector<TexturePathAndMetadata> tempTexData;

		for (AssimpWrapper::TEX_TYPE_ROLE textureTypeRolePair : AssimpWrapper::ASSIMP_TEX_TYPES)
		{
			GetTexMetaData(aiMat, textureTypeRolePair, tempTexData);
		}

		return MatMetaData{ tempTexData };
	}


	static void GetTexMetaData(const aiMaterial* aiMat, const AssimpWrapper::TEX_TYPE_ROLE textureTypeRolePair, std::vector<TexturePathAndMetadata>& texPathAndMetaData)
	{
		auto& [aiType, role] = textureTypeRolePair;

		auto texCountByType = aiMat->GetTextureCount(aiType);

		for (uint32_t i = 0u; i < texCountByType; ++i)
		{
			aiString aiTexPath;
			uint32_t uvIndex = 0u;
			aiTextureMapMode aiMapModes[3]{ aiTextureMapMode_Wrap, aiTextureMapMode_Wrap, aiTextureMapMode_Wrap };

			aiMat->GetTexture(aiType, i, &aiTexPath, nullptr, &uvIndex, nullptr, nullptr, &aiMapModes[0]);

			std::array<SamplingMode, 3> mapModes;
			for (uint32_t j = 0; j < 3; ++j)
			{
				mapModes[j] = AssimpWrapper::TEXMAPMODE_MAP.at(aiMapModes[j]);
			}

			texPathAndMetaData.push_back(
				{
					aiTexPath.C_Str(),
					{
						role,
						mapModes,
						static_cast<uint8_t>(uvIndex),
						0u
					}
				});
		}
	}


	static RawTextureData GetRawTextureData(const aiScene* scene, const std::string& modelPath, const std::string& texPath)
	{
		const char* texName = aiScene::GetShortFilename(texPath.c_str());

		const aiTexture* aiTex = scene->GetEmbeddedTexture(texPath.c_str());

		bool embedded = (aiTex != nullptr);

		if (embedded)
		{
			UINT len = aiTex->mHeight == 0 ? aiTex->mWidth : aiTex->mHeight * aiTex->mWidth;
			return { texPath, std::unique_ptr<char[]>(reinterpret_cast<char*>(aiTex->pcData)), len, true };
		}

		// Texture is not embedded, search for external file
		std::string modelFolderPath = std::filesystem::path(modelPath).parent_path().string();
		std::string absTexPath = modelFolderPath + "/" + texPath;

		if (!std::filesystem::exists(absTexPath))
		{
			std::filesystem::directory_entry texFile;
			if (FileUtils::findFile(modelFolderPath, texName, texFile))
			{
				absTexPath = texFile.path().string();
			}
			else // Path is faulty, try to find it under model directory
			{
				return { texPath, Blob{}, false };
			}
		}

		return { texPath, FileUtils::readAllBytes(absTexPath.c_str()), false };
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


	static std::vector<RawTextureData> LoadRawTextureData(const aiScene* scene, const std::string& modelPath, ID3D11Device* device, const std::map<std::string, std::shared_ptr<Texture>>& uniqueTextures)
	{
		std::vector<RawTextureData> rawTextureData;
		rawTextureData.reserve(uniqueTextures.size());

		std::mutex rawTextureDataMutex;

		std::for_each(std::execution::par, uniqueTextures.begin(), uniqueTextures.end(),
			[&scene, &modelPath, &device, &rawTextureData, &rawTextureDataMutex](const std::pair<std::string, std::shared_ptr<Texture>>& pair)
			{
				auto& [texturePath, tex] = pair;

				RawTextureData rawTextureDatum = MatImporter::GetRawTextureData(scene, modelPath, texturePath);

				if (rawTextureDatum.blob._size == 0)	// Skip textures that were not found, for now.
					return;

				tex->loadFromMemory(reinterpret_cast<unsigned char*>(rawTextureDatum.blob._data.get()), rawTextureDatum.blob._size);
				tex->setUpAsResource(device);

				rawTextureDataMutex.lock();
				rawTextureData.push_back(std::move(rawTextureDatum));
				rawTextureDataMutex.unlock();
			});

		return rawTextureData;
	}
};