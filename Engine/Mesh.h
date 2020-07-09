#pragma once

#include <string>
#include <vector>

#include "Resource.h"
#include "VBuffer.h"
#include "IBuffer.h"
#include "Material.h"
#include "Math.h"

#include "MeshDataStructs.h"
#include "Hull.h"
#include "Geometry.h"

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>


typedef unsigned int UINT;


namespace Procedural { class Terrain; }


// @TODO put these somewhere nice
namespace cereal
{
	template<class Archive>
	void serialize(Archive& archive, SMatrix& m)
	{
		archive(m.m);
	}

	template<class Archive>
	void serialize(Archive& archive, SVec2& vec2)
	{
		archive(vec2.x, vec2.y);
	}

	template<class Archive>
	void serialize(Archive& archive, SVec3& vec3)
	{
		archive(vec3.x, vec3.y, vec3.z);
	}

	template<class Archive>
	void serialize(Archive& archive, Vert3D& v)
	{
		archive(v.pos, v.texCoords, v.normal, v.tangent);
	}

	template<class Archive>
	void serialize(Archive& archive, BonedVert3D& v)
	{
		archive(v.pos, v.texCoords, v.normal, v.boneData);
	}
}


class Mesh : public Resource	//, public SerializableAsset
{
	friend class Model;
public:
	//vertices and indices should be cleared after pushing to the gpu, leaving only the vector memory cost
	std::vector<Vert3D>	_vertices;
	std::vector<UINT> _indices;
	std::vector<Texture> _textures;	//@TODO not sure what to do with this... who should own them?

	SMatrix _transform;

	Material _baseMaterial;	//should be loaded from assimp or otherwise as default... for fallback at least

	//handles to GPU data abstracted in my own classes (useful if I ever get to supporting multiple API-s)
	VBuffer _vertexBuffer;
	IBuffer _indexBuffer;



	// Useful constructors... but @TODO make a material instead of textures!
	Mesh();
	Mesh(std::vector<Vert3D> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, ID3D11Device* device);
	~Mesh();

	// Not so sure, seems like heavy coupling for no reason really!
	Mesh(const SVec2& pos, const SVec2& size, ID3D11Device* device, float z = 0);	//this is used for the screen quads...
	Mesh(const Procedural::Geometry& g, ID3D11Device* device, bool setUp = true, bool hasTangents = true);
	Mesh(const Procedural::Terrain& terrain, ID3D11Device* device);
	Mesh(const Hull* hull, ID3D11Device* device);


	//@TODO - pull D3D11_BUFFER_DESC from a parameter?
	bool setupMesh(ID3D11Device* device); //, D3D11_BUFFER_DESC vertexBufferDesc, D3D11_BUFFER_DESC indexBufferDesc);



	void draw(ID3D11DeviceContext* dc)
	{
		//update and set cbuffers
		_baseMaterial.getVS()->updateBuffersAuto(dc, *this);
		_baseMaterial.getVS()->setBuffers(dc);

		_baseMaterial.getPS()->updateBuffersAuto(dc, *this);
		_baseMaterial.getPS()->setBuffers(dc);


		//set shaders and similar geebees
		dc->IASetInputLayout(_baseMaterial.getVS()->_layout);
		dc->VSSetShader(_baseMaterial.getVS()->_vsPtr, NULL, 0);
		dc->PSSetShader(_baseMaterial.getPS()->_psPtr, NULL, 0);
		dc->PSSetSamplers(0, 1, &_baseMaterial.getPS()->_sState);

		_baseMaterial.bindTextures(dc);

		//could sort by this as well... should be fairly uniform though
		dc->IASetPrimitiveTopology(_baseMaterial.primitiveTopology);

		//these have to change each time unless I'm packing multiple meshes per buffer... can live with that tbh
		dc->IASetVertexBuffers(0, 1, _vertexBuffer.ptr(), &_vertexBuffer._stride, &_vertexBuffer._offset);
		dc->IASetIndexBuffer(_indexBuffer.ptr(), DXGI_FORMAT_R32_UINT, 0);

		dc->DrawIndexed(_indexBuffer.getIdxCount(), 0, 0);
	}



	inline UINT getStride() const { return _vertexBuffer._stride; }



	inline UINT getOffset() const { return _vertexBuffer._offset; }



	template<class Archive>
	void serialize(Archive& archive, UINT matID)
	{
		archive(_indices.size(), _vertices.size(), matID, _transform, _indices, _vertices);
	}



	/* 
	// Old way, cumbersome and inflexible... and I cba writing an entire library for this
	// to do it properly so will use Cereal (boost is a huge dependency)
	MemChunk Serialize() //override
	{
		// Header data - fixed size
		UINT indexCount = _indices.size();
		UINT vertexCount = _vertices.size();
		UINT matID = 0u;
		UINT headerSize = 3 * 4 + 64;

		// Content data - variable size
		UINT ibs = indexCount * sizeof(UINT);
		UINT vbs = vertexCount * sizeof(Vert3D);	// @TODO change when vertex changes
		UINT dataSize = ibs + vbs;

		UINT totalSize = headerSize + dataSize;

		UINT offset = 0u;
		MemChunk byterinos(totalSize);

		byterinos.add(&indexCount, offset);
		byterinos.add(&vertexCount, offset);
		byterinos.add(&matID, offset);
		byterinos.add(&_transform, offset);

		byterinos.add(_indices, offset);
		byterinos.add(_vertices, offset);

		if (!byterinos.isFull(offset))
		{
			OutputDebugStringA("MESH SERIALIZATION WARNING: SIZE MISMATCH!");
			exit(7646);
		}

		return byterinos;
	}

	void deserialize(MemChunk& memChunk)
	{
		UINT numInds = memChunk.get<UINT>(0);
		UINT numVerts = memChunk.get<UINT>(4);

		_indices.resize(numInds);
		_vertices.resize(numVerts);

		UINT matID = memChunk.get<UINT>(8);
		_transform = memChunk.get<SMatrix>(12);

		UINT indArrSize = sizeof(UINT) * numInds;
		UINT vrtArrSize = sizeof(Vert3D) * numVerts;

		memcpy(_indices.data(), memChunk.get<UINT*>(76), indArrSize);
		memcpy(_vertices.data(), memChunk.get<Vert3D*>(76 + indArrSize), vrtArrSize);
	}*/
};