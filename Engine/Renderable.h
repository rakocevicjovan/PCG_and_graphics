#pragma once
#include "Model.h"
#include "Material.h"
#include "Shader.h"

class Renderable
{
public:
	// Memory heavy but... useful for now I guess?
	SMatrix _localTransform;
	SMatrix _transform;

	Mesh* mesh;

	Material* mat;
	int64_t sortKey;
	float zDepth;

	//uint8_t* _cbufferData;


	Renderable() {}

	Renderable(Mesh& mesh) : mesh(&mesh), mat(&mesh._baseMaterial), _transform(mesh._transform)
	{
		for (Texture& t : mesh._textures)
			mat->_texDescription.push_back({ t._role, &t });
	}



	void Renderable::updateBuffersAuto(ID3D11DeviceContext* cont) const
	{
		mat->getVS()->updateBuffersAuto(cont, *this);
		mat->getPS()->updateBuffersAuto(cont, *this);
	}



	inline void setBuffers(ID3D11DeviceContext* dc) const
	{
		mat->getVS()->setBuffers(dc);
		mat->getPS()->setBuffers(dc);
	}



	//overload after deciding how to sort them
	bool Renderable::operator < (const Renderable& b) const { return sortKey < b.sortKey; }
	
	inline PointLight* getLight() const { return mat->pLight; }
};