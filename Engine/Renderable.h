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
	Mesh* mesh{};
	Material* mat{};
	float zDepth{0.f};

	Renderable() = default;

	Renderable(Mesh& mesh) : mesh(&mesh), mat(mesh._material.get()), _transform(mesh._worldSpaceTransform)
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

	inline const SMatrix& renderTransform() const
	{
		return _transform;
	}
};