#include "SkeletalModel.h"



SkeletalModel::SkeletalModel()
{
}


SkeletalModel::~SkeletalModel()
{
}



SkeletalMesh SkeletalModel::processSkeletalMesh(ID3D11Device* device, aiMesh* mesh, const aiScene* scene, unsigned int ind, SMatrix transform, float rUVx, float rUVy)
{
	// Data to fill
	std::vector<BonedVert3D> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> locTextures;

	bool hasTexCoords = mesh->HasTextureCoords(0);

	float radius = AssimpWrapper::loadVertices(mesh, hasTexCoords, vertices);

	AssimpWrapper::loadIndices(mesh, indices);

	AssimpWrapper::loadMeshMaterial(_path, scene, mesh, locTextures);

	for (Texture& t : locTextures)
		t.SetUpAsResource(device);

	AssimpWrapper::loadBonesAndSkinData(*mesh, vertices, _skeleton, transform);
	// Elision does happen... I was wondering
	return SkeletalMesh(vertices, indices, locTextures, device, transform);
}



bool SkeletalModel::loadModel(ID3D11Device* dvc, const std::string& path, float rUVx, float rUVy)
{
	_path = path;

	assert(FileUtils::fileExists(path) && "File not accessible!");

	unsigned int pFlags =
		aiProcessPreset_TargetRealtime_MaxQuality |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_ConvertToLeftHanded |
		aiProcess_LimitBoneWeights;

	Assimp::Importer importer;

	const aiScene* scene = AssimpWrapper::loadScene(importer, path, pFlags);

	if (!scene)
		return false;
	else
		return loadFromScene(dvc, scene, rUVx, rUVy);
}



bool SkeletalModel::loadFromScene(ID3D11Device* dvc, const aiScene* scene, float rUVx, float rUVy)
{
	_meshes.reserve(scene->mNumMeshes);

	SMatrix rootTransform = AssimpWrapper::aiMatToSMat(scene->mRootNode->mTransformation);
	_skeleton._globalInverseTransform = rootTransform.Invert();

	processNode(dvc, scene->mRootNode, scene, rUVx, rUVy, SMatrix::Identity);

	_skeleton.loadFromAssimp(scene);

	AssimpWrapper::loadAnimations(scene, _anims);

	return true;
}



bool SkeletalModel::processNode(ID3D11Device* dvc, aiNode* node, const aiScene* scene, float rUVx, float rUVy, SMatrix globNodeTransform)
{
	SMatrix locNodeTransform = AssimpWrapper::aiMatToSMat(node->mTransformation);
	globNodeTransform = locNodeTransform * globNodeTransform;

	for (unsigned int i = 0; i < node->mNumMeshes; i++)
		_meshes.emplace_back(processSkeletalMesh(dvc, scene->mMeshes[node->mMeshes[i]], scene, _meshes.size(), globNodeTransform, rUVx, rUVy));

	// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
		this->processNode(dvc, node->mChildren[i], scene, rUVx, rUVy, globNodeTransform);

	return true;
}