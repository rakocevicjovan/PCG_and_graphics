#pragma once
#include "Model.h"

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

	Renderable(Mesh& mesh) : mesh(&mesh), mat(mesh.baseMaterial)
	{
	}



	//overload after deciding how to sort them
	bool Renderable::operator < (const Renderable& b) const
	{
		return sortKey < b.sortKey;
	}

	// this could to be a template methinks...
	//void Renderable::updateDrawData() {}
};