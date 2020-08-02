#pragma once
#include "Mesh.h"
#include "Geometry.h"
#include "Terrain.h"
#include "Hull.h"
#include "MeshLoader.h"



Mesh::Mesh(const Procedural::Terrain& terrain, ID3D11Device* device)
{
	_vertSig._attributes = 
	{
		{VAttribSemantic::POS, VAttribType::FLOAT3, 1u, 0u },
		{VAttribSemantic::TEX_COORD, VAttribType::FLOAT2, 1u, 0u },
		{VAttribSemantic::NORMAL, VAttribType::FLOAT3, 1u, 0u },
		{VAttribSemantic::TANGENT, VAttribType::FLOAT3, 1u, 0u}
	};
	terrain.populateMesh(_vertices, _indices, _textures);
	_transform = SMatrix::CreateTranslation(terrain.getOffset());
	setupMesh(device);
}



Mesh::Mesh(const SVec2& pos, const SVec2& size, ID3D11Device* device, float z)
{
	float originX = (pos.x - size.x * 0.5f) * 2.f;	//[0, 1] -> [-1, 1]
	float originY = (pos.y - size.y * 0.5f) * 2.f;
	float width = size.x * 2.f;
	float height = size.y * 2.f;

	Vert3D topLeft;
	topLeft.pos = SVec3(originX, originY + height, z);
	topLeft.texCoords = SVec2(0.f, 1.f);

	Vert3D topRight;
	topRight.pos = SVec3(originX + width, originY + height, z);
	topRight.texCoords = SVec2(1.f, 1.f);

	Vert3D bottomLeft;
	bottomLeft.pos = SVec3(originX, originY, z);
	bottomLeft.texCoords = SVec2(0.f, 0.f);

	Vert3D bottomRight;
	bottomRight.pos = SVec3(originX + width, originY, z);
	bottomRight.texCoords = SVec2(1.f, 0.f);

	// Redundant attributes here, but not a high priority
	_vertSig._attributes =
	{
		{VAttribSemantic::POS, VAttribType::FLOAT3, 1u, 0u },
		{VAttribSemantic::TEX_COORD, VAttribType::FLOAT2, 1u, 0u },
		{VAttribSemantic::NORMAL, VAttribType::FLOAT3, 1u, 0u },
		{VAttribSemantic::TANGENT, VAttribType::FLOAT3, 1u, 0u}
	};

	_vertices.resize(4 * sizeof(Vert3D));
	memcpy(_vertices.data(), &topLeft, sizeof(Vert3D));
	memcpy(_vertices.data(), &topRight, sizeof(Vert3D));
	memcpy(_vertices.data(), &bottomLeft, sizeof(Vert3D));
	memcpy(_vertices.data(), &bottomRight, sizeof(Vert3D));
	
	_indices = std::vector<unsigned int>{ 0u, 1u, 2u, 2u, 1u, 3u };

	setupMesh(device);
}



Mesh::Mesh(const Procedural::Geometry& g, ID3D11Device* device, bool setUp, bool hasTangents)
{
	/**
	_vertices.reserve(g.positions.size());
	Vert3D v;

	for (int i = 0; i < g.positions.size(); ++i)
	{
		v.pos = g.positions[i];
		v.texCoords = g.texCoords[i];
		v.normal = g.normals[i];
		if(hasTangents)
			v.tangent = g.tangents[i];
		_vertices.push_back(v);
	}

	_indices = g.indices;

	if(setUp)
		setupMesh(device);
	*/
}



Mesh::Mesh(const Hull* hull, ID3D11Device* device)
{
	/*
	const AABB*  aabb = reinterpret_cast<const AABB*>(hull);

	SVec3 sizes = (aabb->maxPoint - aabb->minPoint);
	SVec3 offset = aabb->minPoint + sizes * 0.5f;

	Procedural::Geometry g;
	g.GenBox(sizes);

	_vertices.reserve(g.positions.size());
	Vert3D v;

	for (int i = 0; i < g.positions.size(); ++i)
	{
		v.pos = g.positions[i] + offset;
		v.normal = g.normals[i];
		_vertices.push_back(v);
	}

	_indices = g.indices;

	setupMesh(device);
	*/
}



Mesh::~Mesh()
{
	OutputDebugStringA("MESH DESTRUCT CALLED");
}



void Mesh::loadFromAssimp(const aiScene* scene, ID3D11Device* device, aiMesh* aiMesh, const std::string& path)
{

	std::vector<SVec3> faceTangents;
	faceTangents.reserve(aiMesh->mNumFaces);
	
	_vertSig = MeshLoader::createVertSignature(aiMesh);
	MeshLoader meshLoader;
	meshLoader.loadVertData(_vertSig, _vertices, aiMesh, nullptr);


	// Load the bulky data
	//float radius = AssimpWrapper::loadVertices(aiMesh, hasTexCoords, _vertices);
	_indices.reserve(aiMesh->mNumFaces * 3);
	AssimpWrapper::loadIndices(aiMesh, _indices);

	//AssimpWrapper::loadTangents(aiMesh, _vertices, faceTangents);

	AssimpWrapper::loadMaterial(scene, aiMesh->mMaterialIndex, path, &_baseMaterial, _textures);


	// Not true in the general case... it would require tool support with my own format for this!
	// there is no robust way to infer whether a texture is transparent or not, as some textures use 
	// 4 channels but are fully opaque (aka each pixel has alpha=1) therefore its a mess to sort...
	// brute force checking could solve this but incurs a lot of overhead on load
	// and randomized sampling is not reliable, so for now... we have this
	_baseMaterial._opaque = true;

	for (TextureMetaData& rtp : _baseMaterial._texMetaData)
	{
		rtp._tex = &_textures[reinterpret_cast<UINT>(rtp._tex)];
		rtp._tex->SetUpAsResource(device);
	}
}



bool Mesh::setupMesh(ID3D11Device* device) //, D3D11_BUFFER_DESC vertexBufferDesc, D3D11_BUFFER_DESC indexBufferDesc)
{
	_vertexBuffer = VBuffer(device, _vertices, _vertSig, 0u);
	_indexBuffer = IBuffer(device, _indices);

	//this ABSOLUTELY needs to happen!
	//vertices.clear();
	//indices.clear();

	return true;
}