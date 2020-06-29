#pragma once
#include "Mesh.h"
#include "Geometry.h"
#include "Terrain.h"
#include <iostream>

Mesh::Mesh() {}



Mesh::Mesh(std::vector<Vert3D> verts, std::vector<unsigned int> inds, std::vector<Texture> texes, ID3D11Device* device)
	: _vertices(std::move(verts)), _indices(std::move(inds)), _textures(texes)
{
	//breaks if mesh moves... pretty bad but I shouldn't move it anyways...
	for (auto& t : _textures)
	{
		_baseMaterial._texDescription.push_back({ t._role, &t });
	}

	setupMesh(device);	// Now that we have all the required data, set the vertex buffers and its attribute pointers.
}



Mesh::Mesh(const Procedural::Terrain& terrain, ID3D11Device* device)
{
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

	_vertices.push_back(topLeft);
	_vertices.push_back(topRight);
	_vertices.push_back(bottomLeft);
	_vertices.push_back(bottomRight);

	_indices = std::vector<unsigned int>{ 0u, 1u, 2u, 2u, 1u, 3u };

	setupMesh(device);
}



Mesh::Mesh(const Procedural::Geometry& g, ID3D11Device* device, bool setUp, bool hasTangents)
{
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
}



Mesh::Mesh(const Hull* hull, ID3D11Device* device)
{
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
}



Mesh::~Mesh()
{
	std::cout << "MESH DESTRUCT CALLED" << std::endl;

	//_vertexBuffer->Release();
	//_indexBuffer->Release();
}



bool Mesh::setupMesh(ID3D11Device* device) //, D3D11_BUFFER_DESC vertexBufferDesc, D3D11_BUFFER_DESC indexBufferDesc)
{
	_vertexBuffer = VBuffer(device, _vertices, 0u);
	_indexBuffer = IBuffer(device, _indices);

	//this ABSOLUTELY needs to happen!
	//vertices.clear();
	//indices.clear();

	return true;
}