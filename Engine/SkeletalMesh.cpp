#include "pch.h"
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
	_vertexBuffer = VBuffer(dvc, _vertices.data(), _vertices.size(), _vertSig.getVertByteWidth(), 0u);

	_indexBuffer = IBuffer(dvc, _indices);

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