#include "Model.h"
#include "CollisionEngine.h"
#include "Terrain.h"
#include "FileUtilities.h"
#include "CollisionEngine.h"

Model::Model(const std::string& path)
{
	_path = path;
}



Model::Model(const Collider & collider, ID3D11Device* device)
{
	for each(auto hull in collider.hulls)
		meshes.push_back(Mesh(hull, device));
}



Model::Model(const Procedural::Terrain& terrain, ID3D11Device* device)
{
	meshes.emplace_back(terrain, device);
	transform = SMatrix::CreateTranslation(terrain.getOffset());
}



Model::~Model()
{
}



bool Model::LoadModel(ID3D11Device* device, const std::string& path, float rUVx, float rUVy)
{
	_path = path;

	assert(FileUtils::fileExists(path) && "File does not exist! ...probably.");

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

	//directory = path.substr(0, path.find_last_of('/'));
	//name = path.substr(path.find_last_of('/') + 1, path.size());

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
	std::vector<SVec3> faceTangents;
	std::vector<Texture> locTextures;

	vertices.reserve(mesh->mNumVertices);
	indices.reserve(mesh->mNumFaces * 3);
	faceTangents.reserve(mesh->mNumFaces);

	///THIS COULD BE AN ERROR! WATCH OUT!
	bool hasTexCoords = mesh->mTextureCoords[0];

	Vert3D vertex;

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		//aiVector3D temp = parentTransform * aiVector3D(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		//vertex.pos = SVec3(temp.x, temp.y, temp.z);
		//aiVector3D tempNormals = parentTransform * aiVector3D(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		//vertex.normal = SVec3(tempNormals.x, tempNormals.y, tempNormals.z);

		vertex.pos = SVec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.normal = SVec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		//vertex.normal.Normalize();	not sure if required

		if (hasTexCoords)
			vertex.texCoords = SVec2(mesh->mTextureCoords[0][i].x * rUVx, mesh->mTextureCoords[0][i].y * rUVy);
		else
			vertex.texCoords = SVec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}


	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];

		//populate indices from faces
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
			indices.push_back(face.mIndices[j]);

		faceTangents.push_back(calculateTangent(vertices, face));
	}


	//this is ~ O(n^2) where n = mesh->mNumVertices!!! Horrible way to do it!
	/*for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		SVec3 vertTangent(0.f);
		float found = 0.f;

		for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
		{
			//if face contains the vertex, add the tangent of the face to the vertex
			if (mesh->mFaces[j].mIndices[0] == i || mesh->mFaces[j].mIndices[1] == i || mesh->mFaces[j].mIndices[2] == i)
			{
				vertTangent += faceTangents[j];
				found += 1.0f;
			}
		}

		if (found > 0.0001f)
			vertices[i].tangent = found > 0.0f ? vertTangent /= found : vertTangent;
	}*/

	///better solution, since I'm not using weights anyways I can just normalize it... even if it's not too fast, still better
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace& face = mesh->mFaces[i];	//used only for the name hopefully optimized away... don't need repeated allocation really...
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			vertices[face.mIndices[j]].tangent += faceTangents[i];
		}
	}

	for (Vert3D& vert : vertices)
		vert.tangent.Normalize();
	///end of better solution

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
		aiString obtainedTexturePath;
		mat->GetTexture(type, i, &obtainedTexturePath);

		// Check if texture was loaded before and if so
		int loadedIndex = -1;
		boolean alreadyLoaded = ifTexIsLoaded(obtainedTexturePath, loadedIndex);

		if (alreadyLoaded)
		{
			textures.push_back(textures_loaded[loadedIndex]);
		}
		else
		{
			std::string fPath = _path.substr(0, _path.find_last_of("/\\")) + "/" + std::string(obtainedTexturePath.data);
			Texture texture(device, fPath);
			texture.typeName = typeName;

			bool loaded = texture.Load();	//try to load from file

			if (!loaded)	//didn't work, try to load from memory instead...
			{
				loaded = this->loadEmbeddedTextures(device, textures, scene, fPath, type, typeName);	//for embedded textures

				if (!loaded)
					std::cout << "TEX_LOAD::Texture did not load!" << std::endl;

				return textures;
			}

			textures.push_back(texture);
			textures_loaded.push_back(texture); //@TODO THIS IS DUPLICATING TEXTURES
			// Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
		}
	}

	return textures;
}



bool Model::ifTexIsLoaded(const aiString& texPath, int& index)
{
	for (unsigned int j = 0; j < textures_loaded.size(); ++j)
	{
		if (aiString(textures_loaded[j].fileName) == texPath)
		{
			index = j;
			return true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
		}	
	}
	return false;
}



bool Model::loadEmbeddedTextures(ID3D11Device* device, std::vector<Texture>& textures, const aiScene* scene, std::string& fPath, aiTextureType type, std::string& typeName)
{
	if (scene->HasTextures())
	{
		for (size_t texIndex = 0; texIndex < scene->mNumTextures; texIndex++)
		{

			Texture texture(device, fPath);
			texture.typeName = typeName;

			texture.LoadFromMemory(scene->mTextures[texIndex], device);

			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}

		return true;
	}
	return false;
}


//this allocates a lot... even if it's on the stack, i could probably speed up by reusing the declared SVec3s and SVec2s
SVec3 Model::calculateTangent(const std::vector<Vert3D>& vertices, const aiFace& face)
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