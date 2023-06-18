#include "pch.h"

#include "Mesh.h"
#include "Geometry.h"
#include "Terrain.h"


Mesh::Mesh(const Procedural::Terrain& terrain, ID3D11Device* device)
	: _material(std::make_shared<Material>())
{
	_vertSig._attributes = 
	{
		{VAttribSemantic::POS, VAttribType::FLOAT3, 1u, 0u },
		{VAttribSemantic::TEX_COORD, VAttribType::FLOAT2, 1u, 0u },
		{VAttribSemantic::NORMAL, VAttribType::FLOAT3, 1u, 0u },
		{VAttribSemantic::TANGENT, VAttribType::FLOAT3, 1u, 0u}	// Not sure if true
	};
	terrain.populateMesh(_vertices, _indices);
	//_worldSpaceTransform = SMatrix::CreateTranslation(terrain.getOffset());
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
	memcpy(_vertices.data() + 0 * sizeof(Vert3D), &topLeft, sizeof(Vert3D));
	memcpy(_vertices.data() + 1 * sizeof(Vert3D), &topRight, sizeof(Vert3D));
	memcpy(_vertices.data() + 2 * sizeof(Vert3D), &bottomLeft, sizeof(Vert3D));
	memcpy(_vertices.data() + 3 * sizeof(Vert3D), &bottomRight, sizeof(Vert3D));
	
	_indices = std::vector<unsigned int>{ 0u, 1u, 2u, 2u, 1u, 3u };

	setupMesh(device);
}


// This is bootleg but left for legacy code, can be much better. 
// Actually useful but doesn't belong in mesh class.
Mesh::Mesh(const Procedural::Geometry& g, ID3D11Device* device, bool setUp, bool hasTangents)
{
	_vertSig._attributes =
	{
		{VAttribSemantic::POS, VAttribType::FLOAT3, 1u, 0u },
		{VAttribSemantic::TEX_COORD, VAttribType::FLOAT2, 1u, 0u },
		{VAttribSemantic::NORMAL, VAttribType::FLOAT3, 1u, 0u },
	};
	if (hasTangents)
	{
		_vertSig.addAttribute({ VAttribSemantic::TANGENT, VAttribType::FLOAT3, 1u, 0u });
	}

	uint32_t vertexSize = _vertSig.getVertByteWidth();

	_vertices.resize(g.positions.size() * sizeof(Vert3D));
	Vert3D v;

	for (uint32_t i = 0; i < g.positions.size(); ++i)
	{
		v.pos = g.positions[i];
		v.texCoords = g.texCoords[i];
		v.normal = g.normals[i];
		if(hasTangents)
			v.tangent = g.tangents[i];

		memcpy(_vertices.data() + i * vertexSize, &v, vertexSize);
	}

	_indices = g.indices;

	if(setUp)
		setupMesh(device);
}



bool Mesh::setupMesh(ID3D11Device* device, bool releaseCpuResources)
{
	_vertexBuffer = VBuffer(device, _vertices, _vertSig, 0u);
	_indexBuffer = IBuffer(device, _indices);

	//this ABSOLUTELY needs to happen by default! For now, it's weird
	if (releaseCpuResources)
	{
		_vertices.clear();
		_indices.clear();
	}

	return true;
}