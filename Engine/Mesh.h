#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include "MeshDataStructs.h"
#include "Texture.h"
#include "Math.h"
#include "Shader.h"
#include "ShaderWireframe.h"
#include "ShaderHUD.h"
#include "ShaderDepth.h"
#include "ShaderPT.h"
#include "ShaderShadow.h"
#include "ShaderCM.h"

class Mesh{

	public:

		std::vector<Vert3D> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;	//should be a single texture tbh
		unsigned int indexIntoModelMeshArray;

		//unsigned int VAO, VBO, EBO;
		ID3D11Buffer *_vertexBuffer, *_indexBuffer;

		Mesh() {
			_vertexBuffer = 0;
			_indexBuffer = 0;
		}

		Mesh(std::vector<Vert3D> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, ID3D11Device* device, unsigned int ind)
			: vertices(vertices), indices(indices), textures(textures)
		{		
			_vertexBuffer = 0;
			_indexBuffer = 0;
			indexIntoModelMeshArray = ind;
			setupMesh(device);	// Now that we have all the required data, set the vertex buffers and its attribute pointers.
		}

		Mesh(const SVec2& pos, const SVec2& size, ID3D11Device* device) {

			//make the vertices etc...

			float originX = (pos.x - 0.5) * 2;
			float originY = (pos.y - 0.5) * 2;
			float width = size.x * 2;
			float height = size.y * 2;

			Vert3D topLeft;
			topLeft.pos = SVec3(originX, originY + height, 0.0f);
			topLeft.texCoords = SVec2(0.f, 1.f);
			
			Vert3D topRight;
			topRight.pos = SVec3(originX + width, originY + height, 0.0f);
			topRight.texCoords = SVec2(1.f, 1.f);

			Vert3D bottomLeft;
			bottomLeft.pos = SVec3(originX, originY, 0.0f);
			bottomLeft.texCoords = SVec2(0.f, 0.f);

			Vert3D bottomRight;
			bottomRight.pos = SVec3(originX + width, originY, 0.0f);
			bottomRight.texCoords = SVec2(1.f, 0.f);
			
			vertices.push_back(topLeft);
			vertices.push_back(topRight);
			vertices.push_back(bottomLeft);
			vertices.push_back(bottomRight);
			
			indices = std::vector<unsigned int>{ 0u, 1u, 2u, 1u, 3u, 2u };

			setupMesh(device);
		}



		bool setupMesh(ID3D11Device* device) {

			D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
			D3D11_SUBRESOURCE_DATA vertexData, indexData;
			HRESULT res;

			vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			vertexBufferDesc.ByteWidth = sizeof(Vert3D) * vertices.size();
			vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertexBufferDesc.CPUAccessFlags = 0;
			vertexBufferDesc.MiscFlags = 0;
			vertexBufferDesc.StructureByteStride = 0;

			vertexData.pSysMem = vertices.data();
			vertexData.SysMemPitch = 0;
			vertexData.SysMemSlicePitch = 0;
			
			res = device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer);
			if (FAILED( res ))
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
			if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer)))
				return false;

			return true;
		}



		void draw(ID3D11DeviceContext* dc, Shader& s) {

			unsigned int stride = sizeof(Vert3D);
			unsigned int offset = 0;

			dc->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
			dc->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			dc->PSSetSamplers(0, 1, &s.m_sampleState );
			s.ReleaseShaderParameters(dc);
			dc->PSSetShaderResources(0, 1, &(textures[0].srv));
			dc->DrawIndexed(indices.size(), 0, 0);
		}



		void draw(ID3D11DeviceContext* dc, WireframeShader& s) {

			unsigned int stride = sizeof(Vert3D);
			unsigned int offset = 0;

			dc->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
			dc->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			dc->DrawIndexed(indices.size(), 0, 0);
		}



		void draw(ID3D11DeviceContext* dc, ShaderHUD& s) {

			unsigned int stride = sizeof(Vert3D);
			unsigned int offset = 0;

			dc->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
			dc->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			dc->PSSetSamplers(0, 1, &s.m_sampleState);
			dc->DrawIndexed(indices.size(), 0, 0);
		}



		void draw(ID3D11DeviceContext* dc, ShaderDepth& s) {

			unsigned int stride = sizeof(Vert3D);
			unsigned int offset = 0;

			dc->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
			dc->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			dc->DrawIndexed(indices.size(), 0, 0);
		}



		void draw(ID3D11DeviceContext* dc, ShaderPT& s) {

			unsigned int stride = sizeof(Vert3D);
			unsigned int offset = 0;

			dc->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
			dc->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			dc->DrawIndexed(indices.size(), 0, 0);
		}



		void draw(ID3D11DeviceContext* dc, ShaderShadow& s) {

			unsigned int stride = sizeof(Vert3D);
			unsigned int offset = 0;

			dc->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
			dc->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			dc->DrawIndexed(indices.size(), 0, 0);
		}



		void draw(ID3D11DeviceContext* dc, ShaderCM& s) {

			unsigned int stride = sizeof(Vert3D);
			unsigned int offset = 0;

			dc->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
			dc->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			dc->PSSetSamplers(0, 1, &s.m_sampleState);
			dc->DrawIndexed(indices.size(), 0, 0);
		}
	};















/**/
	class BonedMesh : public Mesh{

	public:

		std::vector<BonedVert> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;
		std::vector<Joint> joints;

		unsigned int VAO, VBO, EBO;

		BonedMesh() {}

		BonedMesh(std::vector<BonedVert> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, std::vector<Joint> joints)
			: vertices(vertices), indices(indices), textures(textures), joints(joints)
		{
			//setupMesh();	// Now that we have all the required data, set the vertex buffers and its attribute pointers.
		}

	};