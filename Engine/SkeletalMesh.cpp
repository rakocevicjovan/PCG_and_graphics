#include "SkeletalMesh.h"
#include "AssimpWrapper.h"
#include "MeshLoader.h"


void SkeletalMesh::loadFromAssimp(const aiScene* scene, ID3D11Device* device, aiMesh* aiMesh, Skeleton& skeleton, const std::string& path)
{
	_vertSig = MeshLoader::createVertSignature(aiMesh);

	//float radius = AssimpWrapper::loadVertices(aiMesh, hasTexCoords, _vertices);
	MeshLoader meshLoader;
	meshLoader.loadVertData(_vertSig, _vertices, aiMesh, &skeleton);

	AssimpWrapper::loadIndices(aiMesh, _indices);

	// Use this index to associate the mesh material with the loaded material.
	UINT matIndex = aiMesh->mMaterialIndex;

	// This doesn't belong here!
	AssimpWrapper::loadMaterial(scene, matIndex, path, &_baseMaterial, _textures);

	for (Texture& t : _textures)
		t.SetUpAsResource(device);

	_baseMaterial._opaque = true;

	for (TextureMetaData& rtp : _baseMaterial._texMetaData)
	{
		rtp._tex = &_textures[reinterpret_cast<UINT>(rtp._tex)];
		rtp._tex->SetUpAsResource(device);
	}
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
	//update and set cbuffers
	_baseMaterial.getVS()->updateBuffersAuto(dc, *this);
	_baseMaterial.getVS()->setBuffers(dc);

	_baseMaterial.getPS()->updateBuffersAuto(dc, *this);
	_baseMaterial.getPS()->setBuffers(dc);


	//set shaders and similar geebees
	dc->IASetInputLayout(_baseMaterial.getVS()->_layout);
	dc->VSSetShader(_baseMaterial.getVS()->_vsPtr, NULL, 0);
	dc->PSSetShader(_baseMaterial.getPS()->_psPtr, NULL, 0);
	_baseMaterial.bindSamplers(dc);
	_baseMaterial.bindTextures(dc);

	//could sort by this as well... should be fairly uniform though
	dc->IASetPrimitiveTopology(_baseMaterial._primitiveTopology);

	//these have to change each time unless I'm packing multiple meshes per buffer... can live with that tbh
	dc->IASetVertexBuffers(0, 1, _vertexBuffer.ptr(), &_vertexBuffer._stride, &_vertexBuffer._offset);
	dc->IASetIndexBuffer(_indexBuffer.ptr(), DXGI_FORMAT_R32_UINT, 0);

	dc->DrawIndexed(_indexBuffer.getIdxCount(), 0, 0);
}