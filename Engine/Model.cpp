#include "Model.h"
#include "CollisionEngine.h"
#include "Terrain.h"
#include "FileUtilities.h"
#include "Collider.h"


Model::Model(const std::string& path)
{
	_path = path;
}



Model::Model(const Collider & collider, ID3D11Device* device)
{
	for each(auto hull in collider.getHulls())
		_meshes.push_back(Mesh(hull, device));
}



Model::Model(const Procedural::Terrain& terrain, ID3D11Device* device)
{
	_meshes.emplace_back(terrain, device);
	_transform = SMatrix::CreateTranslation(terrain.getOffset());
}



Model::~Model()
{
	if(collider) delete collider;
}



bool Model::LoadModel(ID3D11Device* device, const std::string& path, float rUVx, float rUVy)
{
	_path = path;

	assert(FileUtils::fileExists(path) && "File does not exist! ...probably.");

	UINT pFlags =
		aiProcessPreset_TargetRealtime_MaxQuality |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs |
		aiProcess_PreTransformVertices |
		aiProcess_ConvertToLeftHanded;

	Assimp::Importer importer;

	const aiScene* scene = AssimpWrapper::loadScene(importer, path, pFlags);

	if (!scene)
	{
		return false;
	}
		

	_meshes.reserve(scene->mNumMeshes);

	processNode(device, scene->mRootNode, scene, scene->mRootNode->mTransformation, rUVx, rUVy);

	return true;
}



bool Model::processNode(ID3D11Device* device, aiNode* node, const aiScene* scene, aiMatrix4x4 parentTransform, float rUVx, float rUVy)
{
	aiMatrix4x4 concatenatedTransform = parentTransform * node->mTransformation;	//or reversed! careful!
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		unsigned int ind = _meshes.size();
		_meshes.emplace_back();
		processMesh(device, mesh, _meshes.back(), scene, ind, concatenatedTransform, rUVx, rUVy);
		_meshes.back().setupMesh(device);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(device, node->mChildren[i], scene, concatenatedTransform, rUVx, rUVy);
	}

	return true;
}



bool Model::processMesh(ID3D11Device* device, aiMesh* aiMesh, Mesh& mesh, const aiScene *scene, unsigned int ind, aiMatrix4x4 parentTransform, float rUVx, float rUVy)
{
	mesh._vertices.reserve(aiMesh->mNumVertices);
	mesh._indices.reserve(aiMesh->mNumFaces * 3);

	std::vector<SVec3> faceTangents;
	faceTangents.reserve(aiMesh->mNumFaces);

	bool hasTexCoords = aiMesh->HasTextureCoords(0);
	bool hasNormals = aiMesh->HasNormals();
	bool hasTangents = aiMesh->HasTangentsAndBitangents();

	float radius = AssimpWrapper::loadVertices(aiMesh, hasTexCoords, mesh._vertices);

	AssimpWrapper::loadIndices(aiMesh, mesh._indices);	// also calculates tangents per face

	AssimpWrapper::loadTangents(aiMesh, mesh._vertices, faceTangents);

	AssimpWrapper::loadMaterials(_path, scene, aiMesh, mesh._textures);

	
	// Not true in the general case... it would require tool support with my own format for this!
	// there is no robust way to infer whether a texture is transparent or not, as some textures use 
	// 32 bits but are fully opaque (aka each pixel has alpha=1) therefore its a mess to sort...
	// brute force checking could solve this but incurs a lot of overhead on load
	// and randomized sampling is not reliable, so for now... we have this
	mesh._baseMaterial._opaque = true;

	// We got through import shenanigans, these textures are valid and will be uploaded to the gpu
	for (Texture& t : mesh._textures)
	{
		t.SetUpAsResource(device);
		mesh._baseMaterial._texDescription.push_back({ t._role, &t });

		//we can at least know it's transparent if it has an opacity map, better than nothing
		if (t._role == OPACITY)
			mesh._baseMaterial._opaque = false;
	}

	return true;
}