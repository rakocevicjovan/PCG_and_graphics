#pragma once
#include "FileUtilities.h"
#include "Texture.h"
#include "Animation.h"

#pragma warning(push)
#pragma warning(disable : 26495)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma warning(pop)

class SkModel;
class Material;
class Mesh;

class AssimpWrapper
{
public:

	typedef std::pair<aiTextureType, TextureRole> TEX_TYPE_ROLE;

	static inline const std::vector<TEX_TYPE_ROLE> ASSIMP_TEX_TYPES
	{
		{ aiTextureType_DIFFUSE,			DIFFUSE },
		{ aiTextureType_NORMALS,			NORMAL },
		{ aiTextureType_SPECULAR,			SPECULAR },
		{ aiTextureType_SHININESS,			SHININESS },
		{ aiTextureType_OPACITY,			OPACITY },
		//{ aiTextureType_EMISSIVE,			EMISSIVE },
		{ aiTextureType_DISPLACEMENT,		DPCM },
		{ aiTextureType_LIGHTMAP,			AMB_OCCLUSION },
		{ aiTextureType_REFLECTION,			REFLECTION },
		// PBR
		{ aiTextureType_BASE_COLOR,			REFRACTION },
		//{ aiTextureType_EMISSION_COLOR,	EMISSIVE },
		{ aiTextureType_METALNESS,			METALLIC },
		{ aiTextureType_DIFFUSE_ROUGHNESS,	ROUGHNESS },
		{ aiTextureType_AMBIENT_OCCLUSION,	AMB_OCCLUSION },
		// Mystery meat
		{ aiTextureType_UNKNOWN,			OTHER }
	};

	inline static const std::map<aiTextureMapMode, SamplingMode> TEXMAPMODE_MAP
	{
		{aiTextureMapMode_Wrap,		SamplingMode::WRAP},
		{aiTextureMapMode_Clamp,	SamplingMode::CLAMP},
		{aiTextureMapMode_Decal,	SamplingMode::BORDER},
		{aiTextureMapMode_Mirror,	SamplingMode::MIRROR}
	};


	static const aiScene* loadScene(Assimp::Importer& importer, const std::string& path, UINT pFlags);

	static void loadIndices(aiMesh* aiMesh, std::vector<UINT>& indices)
	{
		indices.reserve(static_cast<size_t>(aiMesh->mNumFaces * 3));

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

	static std::vector<std::string> loadTextureNames(const aiScene* scene);

	static bool loadEmbeddedTexture(Texture& texture, const aiScene* scene, const char* str);

	static void ImportAnimations(const aiScene* scene, std::vector<Animation>& outAnims);

	static void loadAllBoneNames(const aiScene* scene, aiNode* node, std::set<std::string>& boneNames);

	// Helpers

	// This should be done by assimp internally now. If the metadata is missing, this will not help...
	static void correctAxes(const aiScene* aiScene);

	static std::vector<aiString> getExtTextureNames(const aiScene* scene);

	static SVec3 calcFaceTangent(const std::vector<Vert3D>& vertices, const aiFace& face);

	static aiNode* findModelNode(aiNode* node, SMatrix& meshRootTransform);

	static SMatrix getNodeGlobalTransform(const aiNode* node);

	static bool containsRiggedMeshes(const aiScene* scene);

	static UINT countChildren(const aiNode* node);

	inline static D3D11_PRIMITIVE_TOPOLOGY getPrimitiveTopology(const aiMesh* mesh)
	{
		switch (mesh->mPrimitiveTypes)
		{
			case aiPrimitiveType_LINE:
				return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			case aiPrimitiveType_POINT:
				return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
			case aiPrimitiveType_TRIANGLE:
				return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			default: 
				OutputDebugStringA("Unexpected primitive topology found by assimp. Investigate.");
				return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}

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