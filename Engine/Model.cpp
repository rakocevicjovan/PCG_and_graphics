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
	Material myMat;

	std::vector<SVec3> faceTangents;

	vertices.reserve(mesh->mNumVertices);
	indices.reserve(mesh->mNumFaces * 3);
	faceTangents.reserve(mesh->mNumFaces);

	bool hasTexCoords = mesh->mTextureCoords[0];
	float maxDist = 0.f;
	Vert3D vertex;

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		//memcpy faster?
		vertex.pos = SVec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		
		float curDist = vertex.pos.LengthSquared();
		if (maxDist < curDist)
			maxDist = curDist;

		vertex.normal = SVec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		//vertex.normal.Normalize();	//not sure if required, should be so already

		vertex.texCoords = hasTexCoords ? SVec2(mesh->mTextureCoords[0][i].x * rUVx, mesh->mTextureCoords[0][i].y * rUVy) : SVec2::Zero;

		vertices.push_back(vertex);
	}

	maxDist = sqrt(maxDist);


	aiFace face;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		 face = mesh->mFaces[i];

		//populate indices from faces
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
			indices.push_back(face.mIndices[j]);

		//calculate tangents for faces
		faceTangents.push_back(calculateTangent(vertices, face));
	}


	//even if it's not too fast, this is still a better solution to the previous one (bottom)
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		face = mesh->mFaces[i];	//used only for the name hopefully optimized away... don't need repeated allocation really...
		
		//assign face tangents to vertex tangents
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
			vertices[face.mIndices[j]].tangent += faceTangents[i];
	}

	//after this step we have all the tangents properly calculated
	for (Vert3D& vert : vertices)
		vert.tangent.Normalize();


	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// Diffuse maps
		std::vector<Texture> diffuseMaps = loadMaterialTextures(device, scene, material, aiTextureType_DIFFUSE, "texture_diffuse");
		locTextures.insert(locTextures.end(), diffuseMaps.begin(), diffuseMaps.end());

		// Specular maps
		std::vector<Texture> specularMaps = loadMaterialTextures(device, scene, material, aiTextureType_SPECULAR, "texture_specular");
		locTextures.insert(locTextures.end(), specularMaps.begin(), specularMaps.end());

		// Normal maps
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

		//what? this returns whatever I gave it doesn't it???
		std::string fPath = _path.substr(0, _path.find_last_of("/\\")) + "/" + std::string(obtainedTexturePath.data);
		Texture texture(device, fPath);
		texture.typeName = typeName;

		//try to load this texture from file
		bool loaded = texture.LoadFromStoredPath();	

		//load from file failed - probably means it is embedded, try to load from memory instead...
		if (!loaded)
			loaded = this->loadEmbeddedTextures(device, textures, scene, fPath, type, typeName);

		//load failed completely - most likely the data is corrupted or my library doesn't support it
		if (!loaded)
		{
			std::cout << "TEX_LOAD::Texture did not load!" << std::endl;	//@TODO use logger here instead
			continue;
		}

		textures.push_back(texture);
	}

	return textures;
}



bool Model::loadEmbeddedTextures(ID3D11Device* device, std::vector<Texture>& textures, const aiScene* scene, std::string& fPath, aiTextureType type, std::string& typeName)
{
	if (scene->HasTextures())
	{
		for (size_t texIndex = 0; texIndex < scene->mNumTextures; texIndex++)
		{
			Texture texture(device, fPath);
			texture.typeName = typeName;

			size_t texSize = scene->mTextures[texIndex]->mHeight * scene->mTextures[texIndex]->mWidth;
			
			if (texSize == 0)
				texSize = scene->mTextures[texIndex]->mWidth;

			texture.LoadFromMemory(reinterpret_cast<unsigned char*>(scene->mTextures[texIndex]->pcData), texSize, device);

			textures.push_back(texture);
		}

		return true;
	}
	return false;
}


//this allocates unnecessarily? Even if it's on the stack, I could probably speed up by reusing the declared SVec3s and SVec2s
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



//this is ~ O(n^2) where n = mesh->mNumVertices!!! Horrible way to do tangents!
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

/*	part of resource now
	//directory = path.substr(0, path.find_last_of('/'));
	//name = path.substr(path.find_last_of('/') + 1, path.size());
*/

//aiVector3D temp = parentTransform * aiVector3D(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
//vertex.pos = SVec3(temp.x, temp.y, temp.z);
//aiVector3D tempNormals = parentTransform * aiVector3D(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
//vertex.normal = SVec3(tempNormals.x, tempNormals.y, tempNormals.z);