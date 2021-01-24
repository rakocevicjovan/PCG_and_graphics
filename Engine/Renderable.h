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
	float zDepth;

	//uint8_t* _cbufferData;


	Renderable() : mat(nullptr), mesh(nullptr), zDepth(0) {}

	Renderable(Mesh& mesh) : mesh(&mesh), mat(mesh._material.get()), _transform(mesh._transform)
	{}



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
};