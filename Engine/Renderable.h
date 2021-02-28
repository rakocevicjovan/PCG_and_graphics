#pragma once
#include "Model.h"
#include "Material.h"
#include "Shader.h"

/*
struct Renderable2
{
	Mesh* mesh;
	Material* mat;
	SMatrix _transform;

	Renderable2(Mesh& mesh) : mesh(&mesh), mat(mesh._material.get())
	{}


	// Wrong, make material do this. It needs to encapsulate everything. This will become more complex over time.
	void Renderable2::updateBuffersAuto(ID3D11DeviceContext* cont) const
	{
		mat->getVS()->updateBuffersAuto(cont, *this);
		mat->getPS()->updateBuffersAuto(cont, *this);
	}


	// Same as above. Can't allow this to become a god class.
	inline void setBuffers(ID3D11DeviceContext* dc) const
	{
		mat->getVS()->setBuffers(dc);
		mat->getPS()->setBuffers(dc);
	}
};
*/

class Renderable
{
public:
	// Memory heavy but... useful for now I guess?
	SMatrix _localTransform;
	SMatrix _transform;
	Mesh* mesh;
	Material* mat;
	float zDepth;

	Renderable() : mat(nullptr), mesh(nullptr), zDepth(0) {}

	Renderable(Mesh& mesh) : mesh(&mesh), mat(mesh._material.get()), _transform(mesh._transform)
	{}


	// Wrong, make material do this. It needs to encapsulate everything. This will become more complex over time.
	void Renderable::updateBuffersAuto(ID3D11DeviceContext* cont) const
	{
		mat->getVS()->updateBuffersAuto(cont, *this);
		mat->getPS()->updateBuffersAuto(cont, *this);
	}


	// Same as above. Can't allow this to become a god class.
	inline void setBuffers(ID3D11DeviceContext* dc) const
	{
		mat->getVS()->setBuffers(dc);
		mat->getPS()->setBuffers(dc);
	}
};