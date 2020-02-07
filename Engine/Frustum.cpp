#pragma once
#include "Frustum.h"



Frustum::Frustum(const SMatrix& pm)
{
	extractPlanesFromMatrix(pm);

	_fov = 2. * atan(1.0 / pm._22);	//in radians
	_ar = pm._22 / pm._11;

	float r = pm._33;
	float a = pm._43;

	_zn = -a / r;
	_zf = (-r * _zn) / (1. - r);
}


Frustum::Frustum(float fov, float ar, float zn, float zf) : _fov(fov), _ar(ar), _zn(zn), _zf(zf)
{
	extractPlanesFromMatrix(DirectX::XMMatrixPerspectiveFovLH(fov, ar, zn, zf));
}


//basically recreates most of the frustum, could be done by multiplying with the view matrix I guess... but it's fast regardless
void Frustum::update(const SMatrix& vpm)
{
	extractPlanesFromMatrix(vpm);
}