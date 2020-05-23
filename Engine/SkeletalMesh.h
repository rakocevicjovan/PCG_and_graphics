#pragma once

#include "VBuffer.h"
#include "IBuffer.h"
#include <vector>
#include "MeshDataStructs.h"
#include "Math.h"
#include "Geometry.h"
#include "SkelAnimShader.h"
#include "Material.h"


class SkeletalMesh
{
public:

	std::vector<BonedVert3D> _vertices;
	std::vector<UINT> _indices;
	std::vector<Texture> _textures;

	SMatrix _transform;

	Material _baseMaterial;
	std::vector<Bone> joints;

	unsigned int indexIntoModelMeshArray;

	VBuffer _vertexBuffer;
	IBuffer _indexBuffer;


	SkeletalMesh(){}



	SkeletalMesh(std::vector<BonedVert3D> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, ID3D11Device* dvc, unsigned int ind) 
		: _vertices(vertices), _indices(indices), _textures(textures)
	{
		for (auto& t : _textures)
		{
			_baseMaterial._texDescription.push_back({ t._role, &t });
		}

		indexIntoModelMeshArray = ind;
		setupSkeletalMesh(dvc);	// Now that we have all the required data, set the vertex buffers and its attribute pointers.
	}



	bool setupSkeletalMesh(ID3D11Device* dvc)
	{
		D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData, indexData;
		HRESULT res;

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(BonedVert3D) * _vertices.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		vertexData.pSysMem = _vertices.data();
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		_vertexBuffer._stride = sizeof(BonedVert3D);
		_vertexBuffer._offset = 0u;

		res = dvc->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer.ptrVar());
		if (FAILED(res))
			return false;

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(unsigned int) * _indices.size();
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the index data.
		indexData.pSysMem = _indices.data();
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		_indexBuffer.setIdxCount(_indices.size());

		// Create the index buffer.
		if (FAILED(dvc->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer.ptrVar())))
			return false;

		return true;
	}



	inline PointLight* getLight() const { return _baseMaterial.pLight; }


	
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
	
};