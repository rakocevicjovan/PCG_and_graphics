#pragma once
#include "Math.h"
#include <vector>
#include <array>

class Camera;

class Frustum
{
public:
	//left, right, bottom, top, near, far
	std::array<SPlane, 6> _planes;

	float _fov;
	float _ar;
	float _zn;
	float _zf;

	
	Frustum() {}


	Frustum(float fov, float ar, float zn, float zf);

	Frustum(const SMatrix& pm);

	void update(const SMatrix& vpMat);

	void getCorners();


	// @TODO Make logarithmic later, doesnt matter now
	std::vector<float> calcSplitDistances(uint8_t n, float minZ, float maxZ)
	{
		return std::vector<float>(n, (maxZ - minZ) / n);
	}



	std::vector<SMatrix> createCascadeProjMatrices(const Frustum& f, uint8_t n)
	{
		std::vector<SMatrix> result;

		std::vector<float> zees = calcSplitDistances(3, _zn, _zf);
		float currentNearZ = _zn;

		for (int i = 0; i < zees.size(); ++i)
		{
			float currentFarZ = currentNearZ + zees[i];
			result.push_back(DirectX::XMMatrixPerspectiveFovLH(_fov, _ar, currentNearZ, currentFarZ));
			currentNearZ = currentFarZ;
		}
		
		return result;
	}


private:

	//from the fast extraction paper...
	inline void extractPlanesFromMatrix(const SMatrix& vpm)
	{
		//xyzw data representation of a plane, xyz being the normal, w distance to origin

		_planes[0] = { vpm._14 + vpm._11, vpm._24 + vpm._21, vpm._34 + vpm._31, vpm._44 + vpm._41 };
		_planes[1] = { vpm._14 - vpm._11, vpm._24 - vpm._21, vpm._34 - vpm._31, vpm._44 - vpm._41 };
		_planes[2] = { vpm._14 + vpm._12, vpm._24 + vpm._22, vpm._34 + vpm._32, vpm._44 + vpm._42 };
		_planes[3] = { vpm._14 - vpm._12, vpm._24 - vpm._22, vpm._34 - vpm._32, vpm._44 - vpm._42 };
		_planes[4] = { vpm._13, vpm._23, vpm._33, vpm._43 };
		_planes[5] = { vpm._14 - vpm._13, vpm._24 - vpm._23, vpm._34 - vpm._33, vpm._44 - vpm._43 };

		for (SPlane& p : _planes)
			p.Normalize();
	}
};