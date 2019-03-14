#include "Model.h"
#include "CollisionEngine.h"



Model::Model(const std::string& path)
{
	name = path;
}



Model::~Model()
{
	if (collider)
		delete collider;
}



bool Model::LoadModel(ID3D11Device* device, const std::string& path, float rUVx, float rUVy)
{
	assert(fileExists(path) && "File does not exist! ...probably.");

	unsigned int pFlags =
		aiProcessPreset_TargetRealtime_MaxQuality |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs |
		aiProcess_PreTransformVertices |
		aiProcess_ConvertToLeftHanded;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, pFlags);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::string errString("Assimp error:" + std::string(importer.GetErrorString()));
		OutputDebugStringA(errString.c_str());
		return false;
	}

	directory = path.substr(0, path.find_last_of('/'));
	name = path.substr(path.find_last_of('/') + 1, path.size());

	processNode(device, scene->mRootNode, scene, scene->mRootNode->mTransformation, rUVx, rUVy);
	return true;
}



bool Model::processNode(ID3D11Device* device, aiNode* node, const aiScene* scene, aiMatrix4x4 parentTransform, float rUVx, float rUVy)
{

	aiMatrix4x4 concatenatedTransform = parentTransform * node->mTransformation;	//or reversed! careful!
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		unsigned int ind = meshes.size();
		meshes.push_back(processMesh(device, mesh, scene, ind, concatenatedTransform, rUVx, rUVy));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		this->processNode(device, node->mChildren[i], scene, concatenatedTransform, rUVx, rUVy);
	}
	return true;
}



Mesh Model::processMesh(ID3D11Device* device, aiMesh *mesh, const aiScene *scene, unsigned int ind, aiMatrix4x4 parentTransform, float rUVx, float rUVy)
{
	std::vector<Vert3D> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> locTextures;

	bool hasTexCoords = false;

	///THIS COULD BE AN ERROR! WATCH OUT!
	if (mesh->mTextureCoords[0])
		hasTexCoords = true;



	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vert3D vertex;

		//aiVector3D temp = parentTransform * aiVector3D(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		//vertex.pos = SVec3(temp.x, temp.y, temp.z);
		//aiVector3D tempNormals = parentTransform * aiVector3D(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		//vertex.normal = SVec3(tempNormals.x, tempNormals.y, tempNormals.z);

		vertex.pos = SVec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.normal = SVec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		vertex.normal.Normalize();

		if (hasTexCoords)
			vertex.texCoords = SVec2(mesh->mTextureCoords[0][i].x * rUVx, mesh->mTextureCoords[0][i].y * rUVy);
		else
			vertex.texCoords = SVec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}


	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

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

	return Mesh(vertices, indices, locTextures, device, ind);
}



std::vector<Texture> Model::loadMaterialTextures(ID3D11Device* device, const aiScene* scene, aiMaterial *mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture> textures;

	for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString str;
		boolean skip = false;

		mat->GetTexture(type, i, &str);

		// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture			
		for (unsigned int j = 0; j < textures_loaded.size(); j++)
		{
			if (aiString(textures_loaded[j].fileName) == str)
			{
				textures.push_back(textures_loaded[j]);
				skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
				break;
			}
		}

		if (!skip)
		{   // If texture hasn't been loaded already, load it
			std::string fPath = directory + "/" + std::string(str.data);
			Texture texture(device, fPath);
			texture.typeName = typeName;

			//texture.Bind(type);
			bool loaded = texture.Load();

			if (!loaded)
			{
				loaded = this->LoadGLTextures(device, textures, scene, fPath, type, typeName);	//for embedded textures

				if (!loaded)
					std::cout << "Texture did not load!" << std::endl;

				return textures;
			}

			textures.push_back(texture);
			textures_loaded.push_back(texture); //@TODO THIS IS DUPLICATING TEXTURES
			// Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
		}
	}

	return textures;
}



bool Model::LoadGLTextures(ID3D11Device* device, std::vector<Texture>& textures, const aiScene* scene, std::string& fPath, aiTextureType type, std::string& typeName)
{
	if (scene->HasTextures())
	{
		for (size_t ti = 0; ti < scene->mNumTextures; ti++)
		{

			Texture texture(device, fPath);
			texture.typeName = typeName;

			texture.LoadFromMemory(scene->mTextures[ti], device);

			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}

		return true;
	}
	return false;	//has no textures (at least according you ask assimp), so loading none is fine
}



inline bool Model::fileExists(const std::string& name)
{
	std::ifstream f(name.c_str());
	return f.good();
}