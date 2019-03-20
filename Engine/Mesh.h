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
		Mesh(const Procedural::Geometry& g, ID3D11Device* device, bool setUp = true);
		Mesh(Hull* hull, ID3D11Device* device);

		//@todo pull D3D11_BUFFER_DESC out of the function and into the parameter, which will allow flexibility (for instancing) and reuse etc...
		//@todo level - IMPORTANT AS FUCC
		bool setupMesh(ID3D11Device* device);

		void draw(ID3D11DeviceContext* dc, ShaderVolumetric& s);
		void draw(ID3D11DeviceContext* dc, InstancedShader& s);
		void draw(ID3D11DeviceContext* dc, ShaderBase& s);
		void draw(ID3D11DeviceContext* dc, ShaderLight& s);
		void draw(ID3D11DeviceContext* dc, WireframeShader& s);
		void draw(ID3D11DeviceContext* dc, ShaderShadow& s);
		void draw(ID3D11DeviceContext* dc, ShaderHUD& s);
		void draw(ID3D11DeviceContext* dc, ShaderDepth& s);
		void draw(ID3D11DeviceContext* dc, ShaderPT& s);
		void draw(ID3D11DeviceContext* dc, ShaderCM& s);
		void draw(ID3D11DeviceContext* dc, ShaderSkybox& s);
		void draw(ID3D11DeviceContext* dc, ShaderStrife& s);
		void draw(ID3D11DeviceContext* dc, ShaderWater& s);
	};