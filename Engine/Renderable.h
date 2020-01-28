#pragma once
#include "Model.h"
#include "Material.h"
#include "Shader.h"

class Renderable
{
public:
	int64_t sortKey;

	//a bit memory heavy but... useful for now I guess?
	SMatrix transform;
	SMatrix worldTransform;

	Mesh* mesh;
	Material* mat;
	PointLight* pLight;
	float zDepth;

	unsigned char* cbufferdata;

	Renderable() {}
	Renderable(Mesh& mesh) : mesh(&mesh), mat(&mesh._baseMaterial){}



	void updateBuffersAuto(ID3D11DeviceContext* cont) const;



	inline void setBuffers(ID3D11DeviceContext* dc) const
	{
		mat->getVS()->setBuffers(dc);
		mat->getPS()->setBuffers(dc);
	}



	//overload after deciding how to sort them
	bool Renderable::operator < (const Renderable& b) const
	{
		return sortKey < b.sortKey;
	}

	// this could to be a template methinks...
	//void Renderable::updateDrawData() {}
};