#pragma once

#include <d3d11.h>
#include <vector>

#include "MeshDataStructs.h"
#include "Texture.h"
#include "Math.h"
#include "Geometry.h"
#include "SkelAnimShader.h"

class SkeletalMesh
{
public:

	std::vector<BonedVert3D> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	std::vector<Joint> joints;

	unsigned int indexIntoModelMeshArray;

	ID3D11Buffer *_vertexBuffer, *_indexBuffer;

	SkeletalMesh()
	{
	}



	SkeletalMesh(
		std::vector<BonedVert3D> vertices, 
		std::vector<unsigned int> indices, 
		std::vector<Texture> textures, 
		ID3D11Device* dvc, 
		unsigned int ind) : vertices(vertices), indices(indices), textures(textures)
	{
		_vertexBuffer = 0;
		_indexBuffer = 0;
		indexIntoModelMeshArray = ind;
		setupSkeletalMesh(dvc);	// Now that we have all the required data, set the vertex buffers and its attribute pointers.
	}



	bool setupSkeletalMesh(ID3D11Device* dvc)
	{

		D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData, indexData;
		HRESULT res;

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(BonedVert3D) * vertices.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		vertexData.pSysMem = vertices.data();
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		res = dvc->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer);
		if (FAILED(res))
			return false;

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(unsigned int) * indices.size();
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the index data.
		indexData.pSysMem = indices.data();
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		// Create the index buffer.
		if (FAILED(dvc->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer)))
			return false;

		return true;
	}


	/*
	void draw(ID3D11DeviceContext* dc, ShaderSkelAnim& s)
	{
		unsigned int stride = sizeof(BonedVert3D);
		unsigned int offset = 0;

		dc->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
		dc->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		dc->PSSetSamplers(0, 1, &s._sampleState);
		dc->DrawIndexed(indices.size(), 0, 0);
	}
	*/
};