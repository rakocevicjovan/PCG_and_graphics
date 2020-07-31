#pragma once
#include "AssimpWrapper.h"
#include "Mesh.h"
#include "VertSignature.h"

class MeshLoader
{
	VertSignature _vertSig;
	std::vector<uint8_t> _vertPool;

	std::vector<UINT> _indices;
	
	Material* _material;
	std::vector<Texture> _textures;


public:

	void loadFromAssimp(const aiScene* scene, ID3D11Device* device, aiMesh* aiMesh, const std::string& path)
	{
		_vertSig = createVertSignature(aiMesh);

		loadVertData(_vertSig, _vertPool, aiMesh);

		AssimpWrapper::loadIndices(aiMesh, _indices);

		_material = new Material();

		AssimpWrapper::loadMaterial(scene, aiMesh->mMaterialIndex, path, _material, _textures);

		_material->_opaque = true;

		for (TextureMetaData& rtp : _material->_texMetaData)
		{
			rtp._tex = &_textures[reinterpret_cast<UINT>(rtp._tex)];
			rtp._tex->SetUpAsResource(device);
		}
	}



	static VertSignature createVertSignature(aiMesh* aiMesh)
	{
		VertSignature vertSig;

		// Can this even not?
		if (aiMesh->HasPositions())
			vertSig.addAttribute(VAttribSemantic::POS, VAttribType::FLOAT3);

		// Texture coordinates, slightly more involved but still simple, preserves original layout.
		UINT numUVChannels = aiMesh->GetNumUVChannels();

		UINT prevNumComponents = 0u;	// Will never be equal to the actual data the first time.
		VAttribType uvwType;

		for (int i = 0; i < numUVChannels; ++i)
		{
			UINT uvw = aiMesh->mNumUVComponents[i];

			if (uvw != prevNumComponents)
			{
				// The "You are fired." version :V
				//uvwType = (uvw == 1) ? VAttribType::FLOAT : ((uvw == 2) ? VAttribType::FLOAT2 : VAttribType::FLOAT3);

				// Cool but requires FLOAT# enums to be contiguous, which they probably will be regardless
				//uvwType = static_cast<VAttribType>(static_cast<UINT>(VAttribType::FLOAT) + uvw);

				switch (uvw)
				{
				case 1: uvwType = VAttribType::FLOAT;	break;
				case 2: uvwType = VAttribType::FLOAT2;	break;
				case 3: uvwType = VAttribType::FLOAT3;	break;
				}

				vertSig.addAttribute(VAttribSemantic::TEX_COORD, uvwType);
			}
			else
			{
				++(vertSig._attributes.back()._numElements);
			}
			prevNumComponents = uvw;
		}

		// Normals, quite simple.
		if (aiMesh->HasNormals())
			vertSig.addAttribute(VAttribSemantic::NORMAL, VAttribType::FLOAT3);

		// Assimp ensures these two attributes both exist if either does.
		if (aiMesh->HasTangentsAndBitangents())
		{
			vertSig.addAttribute(VAttribSemantic::TANGENT, VAttribType::FLOAT3);
			vertSig.addAttribute(VAttribSemantic::BITANGENT, VAttribType::FLOAT3);
		}

		// For skeletal meshes, this is needed. Doesn't work yet
		if (aiMesh->HasBones())
		{
			vertSig.addAttribute(VAttribSemantic::B_IDX, VAttribType::UINT4);
			vertSig.addAttribute(VAttribSemantic::B_WEIGHT, VAttribType::FLOAT4);
		}

		return vertSig;
	}



	void loadVertData(VertSignature vertSig, std::vector<uint8_t>& vertPool, aiMesh* aiMesh)
	{
		UINT vertByteWidth = vertSig.getVertByteWidth();
		UINT vertPoolSize = vertByteWidth * aiMesh->mNumVertices;

		vertPool.resize(vertPoolSize);	// Memcpy doesn't increase size() so we hackerino
		
		// Pack interleaved, starting with positions
		if (aiMesh->HasPositions())
		{
			UINT posOffset = vertSig.getOffsetOf(VAttribSemantic::POS);
			uint8_t* dst = vertPool.data() + posOffset;
			for (UINT i = 0; i < aiMesh->mNumVertices; ++i)
			{
				memcpy(dst, &aiMesh->mVertices[i], sizeof(aiVector3D));
				dst += vertByteWidth;
			}
		}


		// There are potentially multiple texture coordinate channels, to be stored at an accumulating offset
		// Initial offset is the offset to first channel, all channels will always be contiguous per vertex
		UINT tcOffset = vertSig.getOffsetOf(VAttribSemantic::TEX_COORD);
		UINT numUVChannels = aiMesh->GetNumUVChannels();

		for (UINT i = 0; i < numUVChannels; ++i)
		{
			// Each tc set can have a size of 1, 2 or 3 floats (u, uv, uvw) which is copied every time
			UINT tcByteWidth = aiMesh->mNumUVComponents[i] * sizeof(float);

			// The offset of this texture coordinate channel in the first vertex is specified here
			uint8_t* dst = vertPool.data() + tcOffset;

			for (UINT j = 0; j < aiMesh->mNumVertices; ++j)
			{
				memcpy(dst, &aiMesh->mTextureCoords[i][j], tcByteWidth);
				dst += vertByteWidth;
			}
			// For every new set, we shift the offset again by the size of the previously written set
			tcOffset += tcByteWidth;
		}


		if (aiMesh->HasNormals())
		{
			UINT nrmOffset = vertSig.getOffsetOf(VAttribSemantic::NORMAL);
			uint8_t* dst = vertPool.data() + nrmOffset;
			for (UINT i = 0; i < aiMesh->mNumVertices; ++i)
			{
				memcpy(dst, &aiMesh->mNormals[i], sizeof(aiVector3D));
				dst += vertByteWidth;
			}
		}


		if (aiMesh->HasTangentsAndBitangents())
		{
			UINT tanOffset = vertSig.getOffsetOf(VAttribSemantic::TANGENT);
			uint8_t* dst = vertPool.data() + tanOffset;
			for (UINT i = 0; i < aiMesh->mNumVertices; ++i)
			{
				memcpy(dst, &aiMesh->mTangents[i], sizeof(aiVector3D));
				dst += vertByteWidth;
			}

			UINT btOffset = vertSig.getOffsetOf(VAttribSemantic::BITANGENT);
			dst = vertPool.data() + btOffset;
			for (UINT i = 0; i < aiMesh->mNumVertices; ++i)
			{
				memcpy(dst, &aiMesh->mBitangents[i], sizeof(aiVector3D));
				dst += vertByteWidth;
			}
		}

		// Won't work well until we have bone indices, which means passing a skeleton yada yada
		if (aiMesh->HasBones())
		{
			UINT biOffset = vertSig.getOffsetOf(VAttribSemantic::B_IDX);

			for (UINT i = 0; i < aiMesh->mNumBones; ++i)
			{
				aiBone* aiBone = aiMesh->mBones[i];

				for (UINT j = 0; j < aiBone->mNumWeights; ++j)
				{
					UINT vertID = aiBone->mWeights[j].mVertexId;
					float weight = aiBone->mWeights[j].mWeight;

					uint8_t* dst = vertPool.data() + biOffset + vertID * vertByteWidth;

					// verts[vertID].AddBoneData(boneIndex, weight);
				}
			}
		}
		
	}

};