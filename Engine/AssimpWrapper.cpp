#include "AssimpWrapper.h"
#include "Mesh.h"
#include "VertSignature.h"
#include <array>


// NOT COMPLETED YET, ALSO CHANGE FROM MESH TO SOME INTERMEDIATE TYPE
Mesh* AssimpWrapper::loadMesh(aiMesh* aiMesh)
{
	Mesh* mesh = new Mesh();

	// Generate mesh vertex signature by inspecting the contents of aiMesh.
	VertSignature vertSig;

	// Can this even not?
	if (aiMesh->HasPositions())
		vertSig.addAttribute(VAttribSemantic::POS, VAttribType::FLOAT3);
	
	// Texture coordinates, slightly more involved but quite easy, preserves original layout.
	UINT numUVCH = aiMesh->GetNumUVChannels();

	UINT prevNumComponents = 0;	// Will never be true the first time.
	VAttribType uvwType;

	for (int i = 0; i < numUVCH; ++i)
	{
		UINT uvw = aiMesh->mNumUVComponents[i];

		if (uvw != prevNumComponents)
		{
			// The "You are fired." version :V
			//uvwType = (uvw == 1) ? VAttribType::FLOAT : ((uvw == 2) ? VAttribType::FLOAT2 : VAttribType::FLOAT3);
			
			if (uvw == 1) uvwType = VAttribType::FLOAT;
			else if (uvw == 2) uvwType = VAttribType::FLOAT2;
			else uvwType = VAttribType::FLOAT3;

			vertSig.addAttribute(VAttribSemantic::TEX_COORD, uvwType, 1);
		}
		else
		{
			++(vertSig._attributes.back()._numElements);
		}
	}

	// Normals, quite simple.
	if (aiMesh->HasNormals())
		vertSig.addAttribute(VAttribSemantic::NORMAL, VAttribType::FLOAT3);

	// Assimp ensures that these are either both present or not.
	if (aiMesh->HasTangentsAndBitangents())
	{
		vertSig.addAttribute(VAttribSemantic::TANGENT, VAttribType::FLOAT3);
		vertSig.addAttribute(VAttribSemantic::BITANGENT, VAttribType::FLOAT3);
	}

	return mesh;

	/*
	mesh->_vertices.reserve(aiMesh->mNumVertices);

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
	*/
}



std::vector<Mesh*> AssimpWrapper::loadAllMeshes(aiScene* scene)
{
	UINT numMeshes = scene->mNumMeshes;

	std::vector<Mesh*> result;
	result.reserve(numMeshes);

	for (int i = 0; i < numMeshes; ++i)
	{
		loadMesh(scene->mMeshes[i]);
	}

	return result;
}