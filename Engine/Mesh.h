#pragma once

#include <string>
#include <vector>

#include "Resource.h"
#include "Math.h"
#include "MeshDataStructs.h"
#include "VBuffer.h"
#include "IBuffer.h"
#include "Material.h"
#include "Hull.h"
#include "Geometry.h"


namespace Procedural { class Terrain; }

class Mesh : public Resource
{
protected:

public:
	//vertices and indices should be cleared after pushing to the gpu, leaving only the vector memory cost
	std::vector<Vert3D>	vertices;
	std::vector<unsigned int> indices;
	int indexCount;

	//handles to GPU data abstracted in my own classes (useful if I ever get to supporting multiple API-s)
	VBuffer _vertexBuffer;
	IBuffer _indexBuffer;

	SMatrix _transform;
	Material _baseMaterial;	//should be loaded from assimp or otherwise as default... for fallback at least

	std::vector<Texture> textures;	//@TODO not sure what to do with this... who should own them?
	unsigned int indexIntoModelMeshArray;

	//valid, useful constructors... but @TODO make a material instead of textures!
	Mesh();
	Mesh(std::vector<Vert3D> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, ID3D11Device* device, unsigned int ind);
	~Mesh();
	
	//not so sure, seems like heavy coupling for no reason really!
	Mesh(const SVec2& pos, const SVec2& size, ID3D11Device* device, float z = 0);	//this is used for the screen quads...
	Mesh(const Procedural::Geometry& g, ID3D11Device* device, bool setUp = true, bool hasTangents = true);
	Mesh(const Procedural::Terrain& terrain, ID3D11Device* device);
	Mesh(const Hull* hull, ID3D11Device* device);
	

	//@TODO - pull D3D11_BUFFER_DESC from a parameter?
	bool setupMesh(ID3D11Device* device); //, D3D11_BUFFER_DESC vertexBufferDesc, D3D11_BUFFER_DESC indexBufferDesc);

	inline PointLight* getLight() const { return _baseMaterial.pLight; }

	void draw(ID3D11DeviceContext* dc, PointLight* p)
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

		dc->DrawIndexed(indexCount, 0, 0);
	}


	inline UINT getStride() const { return _vertexBuffer._stride; }

	inline UINT getOffset() const { return _vertexBuffer._offset; }

	//from the old rendering system, but still could be very useful...
	/*
	template <typename FlexibleShaderType>
	void draw(ID3D11DeviceContext* dc, FlexibleShaderType& s)
	{
		unsigned int stride = s.renderFormat.stride;
		unsigned int offset = s.renderFormat.offset;

		for (int i = 0; i < textures.size(); ++i)
			dc->PSSetShaderResources(s.texturesAdded + i, 1, &(textures[i].srv));

		dc->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
		dc->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		dc->IASetPrimitiveTopology(s.renderFormat.primitiveTopology);	
		dc->DrawIndexed(indexCount, 0, 0);
	}

	//special case for instanced shader... this will disappear when I adapt everything to the new system
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

		for (int i = 0; i < textures.size(); ++i)
			dc->PSSetShaderResources(i, 1, &(textures[i].srv));

		dc->DrawIndexedInstanced(indexCount, s._instanceCount, 0, 0, 0);
	}
	*/
};

/*
class MeshInstance
{
public:
	Mesh* mesh;
	Material* mat;
	SMatrix transform;
};
*/