#pragma once

#include <string>
#include <vector>

#include "Resource.h"
#include "VBuffer.h"
#include "IBuffer.h"
#include "Material.h"
#include "Math.h"

#include "MeshDataStructs.h"
#include "AssimpWrapper.h"
#include "VertSignature.h"

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>


typedef unsigned int UINT;
class Hull;
namespace Procedural { class Geometry; class Terrain; }



namespace cereal
{
	template<class Archive>
	void serialize(Archive& archive, Vert3D& v)
	{
		archive(v.pos, v.texCoords, v.normal, v.tangent);
	}
}


class Mesh : public Resource
{
public:

	VertSignature _vertSig;

	//vertices and indices should be cleared after pushing to the gpu, leaving only the vector memory cost
	//std::vector<Vert3D>	_vertices;
	std::vector<uint8_t> _vertices;
	std::vector<UINT> _indices;
	std::vector<Texture> _textures;	//@TODO not sure what to do with this... who should own them?

	SMatrix _transform;

	Material _baseMaterial;	//should be loaded from assimp or otherwise as default... for fallback at least

	//handles to GPU data abstracted in my own classes (useful if I ever get to supporting multiple API-s)
	VBuffer _vertexBuffer;
	IBuffer _indexBuffer;



	// Useful constructors... but @TODO make a material instead of textures!
	Mesh() {};
	~Mesh();

	// Not so sure, seems like heavy coupling for no reason really!
	Mesh(const SVec2& pos, const SVec2& size, ID3D11Device* device, float z = 0);	//this is used for the screen quads...
	Mesh(const Procedural::Geometry& g, ID3D11Device* device, bool setUp = true, bool hasTangents = true);
	Mesh(const Procedural::Terrain& terrain, ID3D11Device* device);
	Mesh(const Hull* hull, ID3D11Device* device);

	void loadFromAssimp(const aiScene* scene, ID3D11Device* device, aiMesh* aiMesh, const std::string& path);


	//@TODO - pull D3D11_BUFFER_DESC from a parameter?
	bool setupMesh(ID3D11Device* device); //, D3D11_BUFFER_DESC vertexBufferDesc, D3D11_BUFFER_DESC indexBufferDesc);



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
		_baseMaterial.bindSamplers(dc);
		_baseMaterial.bindTextures(dc);

		//could sort by this as well... should be fairly uniform though
		dc->IASetPrimitiveTopology(_baseMaterial.primitiveTopology);

		//these have to change each time unless I'm packing multiple meshes per buffer... can live with that tbh
		dc->IASetVertexBuffers(0, 1, _vertexBuffer.ptr(), &_vertexBuffer._stride, &_vertexBuffer._offset);
		dc->IASetIndexBuffer(_indexBuffer.ptr(), DXGI_FORMAT_R32_UINT, 0);

		dc->DrawIndexed(_indexBuffer.getIdxCount(), 0, 0);
	}



	inline UINT getStride() const { return _vertexBuffer._stride; }



	inline UINT getOffset() const { return _vertexBuffer._offset; }



	template<class Archive>
	void serialize(Archive& archive, UINT matID)
	{
		archive(_indices.size(), _vertices.size(), matID, _transform, _indices, _vertices);
	}
};