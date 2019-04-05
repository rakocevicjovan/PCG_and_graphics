#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include "MeshDataStructs.h"
#include "Texture.h"
#include "Math.h"
#include "Geometry.h"
#include "ShaderManager.h"


namespace Procedural { class Terrain; }
class Hull;

class Mesh
{
public:
	std::vector<Vert3D> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	unsigned int indexIntoModelMeshArray;

	ID3D11Buffer *_vertexBuffer = nullptr, *_indexBuffer = nullptr;

	Mesh();
	Mesh(std::vector<Vert3D> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, ID3D11Device* device, unsigned int ind);
	Mesh(const SVec2& pos, const SVec2& size, ID3D11Device* device);	//this is used for the screen quads...
	Mesh(const Procedural::Geometry& g, ID3D11Device* device, bool setUp = true, bool hasTangents = true);
	Mesh(const Procedural::Terrain& terrain, ID3D11Device* device);
	Mesh(Hull* hull, ID3D11Device* device);
	


	//@todo - pull D3D11_BUFFER_DESC from a parameter?
	bool setupMesh(ID3D11Device* device); //, D3D11_BUFFER_DESC vertexBufferDesc, D3D11_BUFFER_DESC indexBufferDesc);

	template <typename FlexibleShaderType>
	void draw(ID3D11DeviceContext* dc, FlexibleShaderType& s)
	{
		unsigned int stride = s.renderFormat.stride;
		unsigned int offset = s.renderFormat.offset;

		for (int i = 0; i < textures.size(); ++i)
		{
			dc->PSSetShaderResources(s.texturesAdded + i, 1, &(textures[i].srv));
		}

		dc->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
		dc->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		dc->IASetPrimitiveTopology(s.renderFormat.primitiveTopology);	
		dc->DrawIndexed(indices.size(), 0, 0);
	}



	template <class InstancedShader>
	void Mesh::draw(ID3D11DeviceContext* dc, InstancedShader& s)
	{
		unsigned int strides[2];
		unsigned int offsets[2];
		ID3D11Buffer* bufferPointers[2];

		strides[0] = sizeof(Vert3D);
		strides[1] = sizeof(InstanceData);

		offsets[0] = 0;
		offsets[1] = 0;

		bufferPointers[0] = _vertexBuffer;
		bufferPointers[1] = s._instanceBuffer;

		dc->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
		dc->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		dc->PSSetSamplers(0, 1, &s._sampleState);
		dc->DrawIndexedInstanced(indices.size(), s._instanceCount, 0, 0, 0);
	}
};