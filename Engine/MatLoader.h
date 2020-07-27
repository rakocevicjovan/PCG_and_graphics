#pragma once
#include "Material.h"
#include "AssimpWrapper.h"


class MatLoader
{
	// Function signatures be wildin
	typedef std::vector<Material*> MatVec;
	typedef std::vector<Texture> TexVec;
public:

	static MatVec LoadAllMaterials(const aiScene* scene, const std::string& path)
	{
		MatVec materials;
		TexVec textures;

		for (UINT i = 0; i < scene->mNumMaterials; ++i)
			materials.emplace_back(LoadMaterial(scene, scene->mMaterials[i], path, textures));
	}



	static Material LoadMaterial(const aiScene* scene, aiMaterial* aiMat, const std::string& path, TexVec textures)
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

		// Textures
		// Diffuse maps
		AssimpWrapper::loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_DIFFUSE, DIFFUSE);

		//  Normal maps
		AssimpWrapper::loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_NORMALS, NORMAL);

		// Specular maps
		AssimpWrapper::loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_SPECULAR, SPECULAR);

		// Shininess maps
		AssimpWrapper::loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_SHININESS, SHININESS);

		// Opacity maps - a bit of a special case, as it indicates that material is potentially transparent
		if (AssimpWrapper::loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_OPACITY, OPACITY))
			mat->_opaque = false;

		// Displacement maps
		AssimpWrapper::loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_DISPLACEMENT, DISPLACEMENT);

		// Ambient occlusion maps
		AssimpWrapper::loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_AMBIENT, AMBIENT);

		// Metallic maps
		AssimpWrapper::loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_METALNESS, METALLIC);

		// Other maps
		AssimpWrapper::loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_UNKNOWN, OTHER);

		// Weird properties... that I never really saw trigger
		AssimpWrapper::loadMaterialTextures(path, textures, scene, aiMat, mat, aiTextureType_NONE, OTHER);
	}

};