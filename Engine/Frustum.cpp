#pragma once
#include "Frustum.h"



Frustum::Frustum(const SMatrix& pm)
{
	SPlane left		(pm._14 + pm._11, pm._24 + pm._21, pm._34 + pm._31, pm._44 + pm._41);
	SPlane right	(pm._14 - pm._11, pm._24 - pm._21, pm._34 - pm._31, pm._44 - pm._41);

	SPlane bottom	(pm._14 + pm._12, pm._24 + pm._22, pm._34 + pm._32, pm._44 + pm._42);
	SPlane top		(pm._14 - pm._12, pm._24 - pm._22, pm._34 - pm._32, pm._44 - pm._42);

	SPlane pnear	(pm._13, pm._23, pm._33, pm._43);
	SPlane pfar		(pm._14 - pm._13, pm._24 - pm._23, pm._34 - pm._33, pm._44 - pm._43);

	planes.push_back(left);
	planes.push_back(right);
	planes.push_back(bottom);
	planes.push_back(top);
	planes.push_back(pnear);
	planes.push_back(pfar);

	for (SPlane& p : planes)
		p.Normalize();
}


void Frustum::update(const SMatrix& pm)
{
	planes[0] = { pm._14 + pm._11, pm._24 + pm._21, pm._34 + pm._31, pm._44 + pm._41 };
	planes[1] = { pm._14 - pm._11, pm._24 - pm._21, pm._34 - pm._31, pm._44 - pm._41 };
	planes[2] = { pm._14 + pm._12, pm._24 + pm._22, pm._34 + pm._32, pm._44 + pm._42 };
	planes[3] = { pm._14 - pm._12, pm._24 - pm._22, pm._34 - pm._32, pm._44 - pm._42 };
	planes[4] = { pm._13, pm._23, pm._33, pm._43 };
	planes[5] = { pm._14 - pm._13, pm._24 - pm._23, pm._34 - pm._33, pm._44 - pm._43 };

	for (SPlane& p : planes)
		p.Normalize();
}