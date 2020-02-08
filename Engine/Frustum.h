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

	// Also works with combined vp matrix to obtain world coordinates
	static std::array<SVec3, 8> extractCorners(const SMatrix& vpMat)
	{
		static const std::array<SVec4, 8> vec4s =
		{
			SVec4(-1, -1,  0, 1.),
			SVec4( 1, -1,  0, 1.),
			SVec4(-1,  1,  0, 1.),
			SVec4( 1,  1,  0, 1.),
			SVec4(-1, -1,  1, 1.),
			SVec4( 1, -1,  1, 1.),
			SVec4(-1,  1,  1, 1.),
			SVec4( 1,  1,  1, 1.)
		};

		//the vp mat works because view is inverted camera matrix anyways... then it inverts back before transforming
		SMatrix inv = vpMat.Invert();

		std::array<SVec3, 8> result;
		for (int i = 0; i < 8; ++i)
		{
			SVec4 temp = SVec4::Transform(vec4s[i], inv);
			SVec3 res(temp.x, temp.y, temp.z);
			result[i] = (res / temp.w);
		}

		return result;
	}


	// @TODO Make logarithmic later, doesnt matter now
	std::vector<float> calcSplitDistances(uint8_t n, float minZ, float maxZ) const
	{
		std::vector<float> result;
		result.reserve(n);

		float d = maxZ - minZ;
		float r = maxZ / minZ;
		float nf = n;

		for (float i = 1; i <= n; ++i)
		{
			//double Zi = Math::lerp(near, far, i / nf);  // Linear increase
			float Zi = Math::lerp(n + (i / nf) * (d - nf), pow(n * (d / nf), i / nf), .5);	//lerp(lin, log, .5), like nvidia

			result.push_back(Zi);
		}

		return result;
		//return std::vector<float>(n, (maxZ - minZ) / n); //all three same
	}



	std::vector<SMatrix> createCascadeProjMatrices(uint8_t n) const
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