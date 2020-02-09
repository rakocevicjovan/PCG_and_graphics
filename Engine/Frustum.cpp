#pragma once
#include "Frustum.h"
#include "Hull.h"
#include "ColFuncs.h"


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


//this can be slightly faster with pmat precombined with vmat (In row major it's vmat*pmat) because vmat is an inverted cmat
std::array<SVec3, 8> Frustum::extractCorners(const SMatrix& pMat)	//, const SMatrix& cMat)
{
	static const std::array<SVec4, 8> vec4s =
	{
		SVec4(-1, -1,  0, 1.),	//nbl
		SVec4(1, -1,  0, 1.),	//nbr
		SVec4(-1,  1,  0, 1.),	//ntl
		SVec4(1,  1,  0, 1.),	//ntr
		SVec4(-1, -1,  1, 1.),	//fbl
		SVec4(1, -1,  1, 1.),	//fbr
		SVec4(-1,  1,  1, 1.),	//ftl
		SVec4(1,  1,  1, 1.) 	//ftr
	};

	SMatrix inv = pMat.Invert();

	std::array<SVec3, 8> result;
	for (int i = 0; i < 8; ++i)
	{
		SVec4 temp = SVec4::Transform(vec4s[i], inv);
		SVec3 res(temp.x, temp.y, temp.z);
		res /= temp.w;
		//res = SVec3::Transform(res, cMat);
		result[i] = res;
	}

	return result;
}



std::vector<float> Frustum::calcSplitDistances(uint8_t n, float minZ, float maxZ) const
{
	// Alternative 1: return std::vector<float>(n, (maxZ - minZ) / n); //all three same
	std::vector<float> result;
	result.reserve(n);

	float d = maxZ - minZ;
	float nf = n;

	for (float i = 1; i <= n; ++i)
	{
		// Alternative 2: double Zi = Math::lerp(near, far, i / nf);  // Linear increase
		float Zi = Math::lerp(n + (i / nf) * (d - nf), pow(n * (d / nf), i / nf), .8);	//lerp(lin, log, .5), like nvidia
		result.push_back(Zi);
	}

	return result;
}



std::vector<SMatrix> Frustum::createCascadeProjMatrices(uint8_t n) const
{
	std::vector<SMatrix> result;

	std::vector<float> zees = calcSplitDistances(3, _zn, _zf);
	float currentNearZ = _zn;

	for (int i = 0; i < zees.size(); ++i)
	{
		float currentFarZ = _zn + zees[i];	// currentNearZ +
		result.push_back(DirectX::XMMatrixPerspectiveFovLH(_fov, _ar, currentNearZ, currentFarZ));
		currentNearZ = currentFarZ;
	}

	return result;
}