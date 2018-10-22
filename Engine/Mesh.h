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

class Mesh{

	public:

		std::vector<Vert3D> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;	//should be a single texture tbh

		//unsigned int VAO, VBO, EBO;
		ID3D11Buffer *_vertexBuffer, *_indexBuffer;

		Mesh() {
			_vertexBuffer = 0;
			_indexBuffer = 0;
		}

		Mesh(std::vector<Vert3D> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, ID3D11Device* device)
			: vertices(vertices), indices(indices), textures(textures)
		{		
			_vertexBuffer = 0;
			_indexBuffer = 0;
			setupMesh(device);	// Now that we have all the required data, set the vertex buffers and its attribute pointers.
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
			if (FAILED( res )){
				return false;
			}


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
			if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer))){
				return false;
			}

			//textures.push_back();

			return true;
		}


		void draw(ID3D11DeviceContext* dc, Shader& s) {

			unsigned int stride = sizeof(Vert3D);
			unsigned int offset = 0;

			// Set the vertex buffer to active in the input assembler so it can be rendered.
			dc->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);

			// Set the index buffer to active in the input assembler so it can be rendered.
			dc->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
			dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			dc->PSSetSamplers(0, 1, &s.m_sampleState );

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