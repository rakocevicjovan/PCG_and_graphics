#pragma once
#include "FileUtilities.h"
#include "Texture.h"
#include "Skeleton.h"
#include "Animation.h"

#include <set>
#include <memory>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class SkeletalModel;
class Material;
class Mesh;


class AssimpWrapper
{
public:

	// New code, trying to do this in a better way
	std::vector<Material*> loadMaterials(aiScene* scene, const std::string& path);

	Mesh* loadMesh(aiMesh* aiMesh);	// Make unique ptrs...

	std::vector<Mesh*> loadAllMeshes(aiScene* scene);


	static const aiScene* loadScene(Assimp::Importer& importer, const std::string& path, UINT pFlags);

	// returns bounding sphere radius
	template <typename VertexType> 
	static float loadVertices(aiMesh* aiMesh, bool hasTexCoords, std::vector<VertexType>& verts)
	{
		verts.reserve(aiMesh->mNumVertices);

		float maxDist = 0.f;
		VertexType vertex;

		for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i)
		{
			vertex.pos = SVec3(&aiMesh->mVertices[i].x);

			vertex.texCoords = hasTexCoords ? SVec2(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y) : SVec2::Zero;

			vertex.normal = SVec3(&aiMesh->mNormals[i].x);	// Should be normalized already?

			verts.push_back(vertex);

			float curDist = vertex.pos.LengthSquared();
			if (maxDist < curDist) maxDist = curDist;
		}

		return sqrt(maxDist);
	}



	static void loadIndices(aiMesh* aiMesh, std::vector<UINT>& indices)
	{
		aiFace face;
		for (UINT i = 0; i < aiMesh->mNumFaces; ++i)
		{
			face = aiMesh->mFaces[i];

			for (UINT j = 0; j < face.mNumIndices; ++j)
				indices.emplace_back(face.mIndices[j]);
		}
	}



	static void loadTangents(aiMesh* aiMesh, std::vector<Vert3D>& verts, std::vector<SVec3>& faceTangents)
	{
		aiFace face;
		for (UINT i = 0; i < aiMesh->mNumFaces; ++i)
		{
			face = aiMesh->mFaces[i];
			faceTangents.emplace_back(calcFaceTangent(verts, face));	// Calculate tangents for faces
		}

		for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i)
		{
			face = aiMesh->mFaces[i];

			for (unsigned int j = 0; j < face.mNumIndices; ++j)			// Assign face tangents to vertex tangents
				verts[face.mIndices[j]].tangent += faceTangents[i];
		}

		for (Vert3D& vert : verts)
			vert.tangent.Normalize();
	}



	static void loadMaterial(const aiScene* scene, UINT index, const std::string& path, Material* mat, std::vector<Texture>& textures);

	static bool loadMaterialTextures(
		const std::string& modelPath,
		std::vector<Texture>& textures,
		const aiScene* scene,
		aiMaterial *aiMat,
		Material* mat,
		aiTextureType aiTexType,
		TextureRole role);

	static bool loadEmbeddedTexture(Texture& texture, const aiScene* scene, aiString* str);

	static void loadAnimations(const aiScene* scene, std::vector<Animation>& outAnims);

	static void loadBonesAndSkinData(const aiMesh& aiMesh, std::vector<BonedVert3D>& verts, Skeleton& skeleton);

	// Seeks upwards from every existing bone, adding intermediate nodes as bones
	static void addMissingBones(Skeleton* skeleton, const aiNode* boneNode, SMatrix meshGlobalMatrix);

	static const aiNode* findSkeletonRoot(const aiNode* node, Skeleton& skeleton, SMatrix pMat);

	static void loadAllBoneNames(const aiScene* scene, aiNode* node, std::set<std::string>& boneNames);


	// Helpers
	static std::vector<aiString> getExtTextureNames(const aiScene* scene);

	static SVec3 calcFaceTangent(const std::vector<Vert3D>& vertices, const aiFace& face);

	static aiNode* findModelNode(aiNode* node, SMatrix& meshRootTransform);

	static SMatrix getNodeGlobalTransform(const aiNode* node);

	static bool containsRiggedMeshes(const aiScene* scene);

	static UINT countChildren(const aiNode* node);


	// Short helpers, inlined
	inline static bool isOnlySkeleton(const aiScene* scene)
	{ return (scene->mNumMeshes == 0); }

	inline static SMatrix calculateOffsetMatrix(const aiNode* boneNode, SMatrix meshGlobalMat)
	{ return meshGlobalMat * getNodeGlobalTransform(boneNode).Invert(); }

	inline static SMatrix aiMatToSMat(const aiMatrix4x4& aiMat)
	{ return SMatrix(&aiMat.a1).Transpose(); }

	inline static SQuat aiQuatToSQuat(const aiQuaternion& aq)
	{ return SQuat(aq.x, aq.y, aq.z, aq.w); }

	inline static void postProcess(Assimp::Importer& i, aiPostProcessSteps f)
	{ i.ApplyPostProcessing(f); }
};