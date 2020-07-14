#include "Model.h"
#include "CollisionEngine.h"
#include "Terrain.h"
#include "FileUtilities.h"
#include "Collider.h"


Model::Model(const std::string& path) : collider(nullptr)
{
	_path = path;
}



Model::Model(const Collider & collider, ID3D11Device* device) : collider(nullptr)
{
	for each(auto hull in collider.getHulls())
		_meshes.push_back(Mesh(hull, device));
}



Model::Model(const Procedural::Terrain& terrain, ID3D11Device* device) : collider(nullptr)
{
	_meshes.emplace_back(terrain, device);
	_transform = SMatrix::CreateTranslation(terrain.getOffset());
}



Model::~Model()
{
	if(collider) delete collider;
}



bool Model::LoadModel(ID3D11Device* device, const std::string& path)
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
		return false;

	return LoadFromScene(device, scene);
}



bool Model::LoadFromScene(ID3D11Device* device, const aiScene* scene)
{
	_meshes.reserve(scene->mNumMeshes);
	processNode(device, scene->mRootNode, scene, scene->mRootNode->mTransformation);
	return true;
}



bool Model::processNode(ID3D11Device* device, aiNode* node, const aiScene* scene, aiMatrix4x4 parentTransform)
{
	aiMatrix4x4 concatenatedTransform = node->mTransformation * parentTransform;

	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		_meshes.emplace_back();
		_meshes.back().loadFromAssimp(scene, device, mesh, concatenatedTransform, _path);
		_meshes.back().setupMesh(device);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(device, node->mChildren[i], scene, concatenatedTransform);
	}

	return true;
}