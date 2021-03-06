#include "SkeletalMesh.h"
#include "AssimpWrapper.h"
#include "MeshLoader.h"


void SkeletalMesh::loadFromAssimp(const aiScene* scene, ID3D11Device* device, aiMesh* aiMesh, 
	std::vector<std::shared_ptr<Material>>& materials, Skeleton* skeleton, const std::string& path)
{
	_vertSig = MeshLoader::createVertSignature(aiMesh);
	MeshLoader meshLoader;
	meshLoader.loadVertData(_vertSig, _vertices, aiMesh, skeleton);
	AssimpWrapper::loadIndices(aiMesh, _indices);

	// Use this index to associate the mesh material with the loaded material.
	_material = materials[aiMesh->mMaterialIndex];
}



bool SkeletalMesh::setupSkeletalMesh(ID3D11Device* dvc)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT res;

	UINT vertByteWidth = _vertSig.getVertByteWidth();	//vertByteWidth * 

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = _vertices.size();	// It's already a pool
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = _vertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	_vertexBuffer._stride = vertByteWidth;
	_vertexBuffer._offset = 0u;

	res = dvc->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer.ptrVar());
	if (FAILED(res))
		return false;

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(UINT) * _indices.size();
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



void SkeletalMesh::draw(ID3D11DeviceContext* dc)
{
	//update and set cbuffers, move to a material function
	_material->getVS()->updateBuffersAuto(dc, *this);
	_material->getVS()->setBuffers(dc);

	_material->getPS()->updateBuffersAuto(dc, *this);
	_material->getPS()->setBuffers(dc);

	//set shaders and similar geebees
	_material->bind(dc);

	//could sort by this as well... should be fairly uniform though
	dc->IASetPrimitiveTopology(_material->_primitiveTopology);

	//these have to change each time unless I'm packing multiple meshes per buffer... can live with that tbh
	dc->IASetVertexBuffers(0, 1, _vertexBuffer.ptr(), &_vertexBuffer._stride, &_vertexBuffer._offset);
	dc->IASetIndexBuffer(_indexBuffer.ptr(), DXGI_FORMAT_R32_UINT, 0);

	dc->DrawIndexed(_indexBuffer.getIdxCount(), 0, 0);
}