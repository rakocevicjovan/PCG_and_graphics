#pragma once
#include "Material.h"
#include "AssimpWrapper.h"


class MatLoader
{
	// Function signatures be wildin
	typedef std::vector<Material*> MatVec;
	typedef std::vector<Texture> TexVec;

	struct TempTexData
	{
		aiString _path;
		TextureMetaData _tmd;
	};

public:

	static MatVec LoadAllMaterials(const aiScene* scene, const std::string& path)
	{
		MatVec materials;
		TexVec textures;

		for (UINT i = 0; i < scene->mNumMaterials; ++i)
			materials.emplace_back(LoadMaterial(scene, scene->mMaterials[i], path, textures));
	}



	static Material LoadMaterial(const aiScene* scene, aiMaterial* aiMat, const std::string& path, TexVec& textures)
	{
		Material* mat = new Material();
		
		// I really don't know what the hell to do with all the properties
		// Could put them in a blob but then not sure how to interpret them...

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
		{
			transparencyFactor = 0.f;
		}

		float shininess;	// Uh which is which...
		if (aiMat->Get(AI_MATKEY_SHININESS, shininess) != aiReturn_SUCCESS)
		{
			shininess = 0.f;
		}

		float specularIntensity;
		if (aiMat->Get(AI_MATKEY_SHININESS_STRENGTH, specularIntensity) != aiReturn_SUCCESS)
		{
			specularIntensity = 0.f;
		}

		float reflectivity;
		if (aiMat->Get(AI_MATKEY_REFLECTIVITY, reflectivity) != aiReturn_SUCCESS)
		{
			reflectivity = 0.f;
		}

		float refractionIndex;	// I guess, who the f uses REFRACTI
		if (aiMat->Get(AI_MATKEY_REFRACTI, refractionIndex) != aiReturn_SUCCESS)
		{
			refractionIndex = 1.f;
		}

		aiColor4D diffuseColour;
		if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColour) != aiReturn_SUCCESS)
		{
			diffuseColour = aiColor4D(.5f);
		}

		aiColor4D ambientColour;
		if (aiMat->Get(AI_MATKEY_COLOR_AMBIENT, ambientColour) != aiReturn_SUCCESS)
		{
			ambientColour = aiColor4D(.5);
		}

		aiColor4D specColour;
		if (aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specColour) != aiReturn_SUCCESS)
		{
			specColour = aiColor4D(.5, .5, .5, 1.);
		}

		aiColor4D emissiveColour;
		if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColour) != aiReturn_SUCCESS)
		{
			emissiveColour = aiColor4D(0., 0., 0., 1.);
		}

		aiColor4D transColour;
		if (aiMat->Get(AI_MATKEY_COLOR_TRANSPARENT, transColour) != aiReturn_SUCCESS)
		{
			transColour = aiColor4D(0.f);
		}

		aiColor4D reflectiveColour;
		if (aiMat->Get(AI_MATKEY_COLOR_REFLECTIVE, reflectiveColour) != aiReturn_SUCCESS)
		{
			reflectiveColour = aiColor4D(0., 0., 0., 1.);
		}


		struct TexTypePair
		{
			aiTextureType _aiType;
			TextureRole role;
		};

		// Textures
		static const std::vector<std::pair< aiTextureType, TextureRole>> ASSIMP_TEX_TYPES
		{
			{ aiTextureType_DIFFUSE, DIFFUSE},
			{ aiTextureType_NORMALS, NORMAL},
			{ aiTextureType_SPECULAR, SPECULAR},
			{ aiTextureType_SHININESS, SHININESS},
			{ aiTextureType_OPACITY, OPACITY},
			//{ aiTextureType_EMISSIVE, EMISSIVE},
			{ aiTextureType_DISPLACEMENT, DISPLACEMENT},
			{ aiTextureType_LIGHTMAP, AMBIENT},
			{ aiTextureType_REFLECTION, REFLECTION},
			// PBR
			{ aiTextureType_BASE_COLOR, REFRACTION},
			//{ aiTextureType_EMISSION_COLOR, EMISSIVE},
			{ aiTextureType_METALNESS, METALLIC},
			{ aiTextureType_DIFFUSE_ROUGHNESS, ROUGHNESS},
			{ aiTextureType_AMBIENT_OCCLUSION, AMBIENT},
			// Mystery meat
			{ aiTextureType_UNKNOWN, OTHER }
		};

		for (int i = 0; i < ASSIMP_TEX_TYPES.size(); ++i)
		{
			AssimpWrapper::loadMaterialTextures(path, textures, scene, aiMat, mat, ASSIMP_TEX_TYPES[i].first, ASSIMP_TEX_TYPES[i].second);
		}

	}



	void loadMetaData(std::vector<TempTexData>& tempTexData,
		aiMaterial *aiMat, aiTextureType aiTexType, TextureRole role)
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
			aiString aiTexPath;
			UINT uvIndex = 0u;
			aiTextureMapMode aiMapModes[3]{ aiTextureMapMode_Wrap, aiTextureMapMode_Wrap , aiTextureMapMode_Wrap };

			aiMat->GetTexture(aiTexType, i, &aiTexPath, nullptr, &uvIndex, nullptr, nullptr, &aiMapModes[0]);

			TextureMapMode mapModes[3];
			for (UINT j = 0; j < 3; ++j)
				mapModes[j] = TEXMAPMODE_MAP.at(aiMapModes[j]);

			tempTexData.push_back(
			{
				aiTexPath,
				{
					nullptr,
					role,
					{ mapModes[0], mapModes[1], mapModes[2] },
					static_cast<uint8_t>(uvIndex),
					0u
				}
			});
		}
	}


	// @TODO only load unique textures to separate container, point from materials
	static std::vector<Texture*> LoadTextures
	(aiScene* scene, const std::string& modelPath, std::vector<TempTexData>& ttdVec)
	{
		std::vector<Texture*> pTexVec;

		for (UINT i = 0; i < ttdVec.size(); ++i)
		{
			TempTexData& ttd = ttdVec[i];

			std::string texName(aiScene::GetShortFilename(ttd._path.C_Str()));

			const aiTexture* aiTex = scene->GetEmbeddedTexture(ttd._path.C_Str());

			Texture* curTex = new Texture();
			curTex->_fileName = texName;

			bool loaded = false;

			// Check if embedded first
			if (aiTex)
			{
				loaded = AssimpWrapper::loadEmbeddedTexture(*curTex, scene, &ttd._path);
			}

			// Not embedded, try to load from file
			if (!loaded)
			{
				// Assumes relative paths
				std::string modelFolderPath = modelPath.substr(0, modelPath.find_last_of("/\\"));
				std::string texPath = modelFolderPath + "/" + std::string(ttd._path.data);

				// If path is faulty, try to find it in the model directory and subdirectories
				if (!std::filesystem::exists(texPath))
				{
					std::filesystem::directory_entry texFile;
					if (FileUtils::findFile(modelFolderPath, texName, texFile))
						texPath = texFile.path().string();
				}

				curTex->_fileName = texPath;	// or texName, I really don't even know why either tbh

				loaded = curTex->LoadFromStoredPath();
			}

			// Load failed completely - most likely the data is corrupted or my library doesn't support it
			if (!loaded)
			{
				OutputDebugStringA("TEX_LOAD::Texture did not load! \n"); //@TODO use logger here instead
				continue;
			}

			ttd._tmd._tex = curTex;
			pTexVec.push_back(curTex);
		}
	}
};