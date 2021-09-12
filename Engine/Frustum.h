#pragma once

class Camera;

// To draw the frustum for debugging just draw a uniform box (dimensions depends on API) using (camView * camProj).Invert() as world mat
class Frustum
{
public:
	//left, right, bottom, top, near, far
	std::array<SPlane, 6> _planes;

	float _fov{ 0.f };
	float _ar{ 0.f };
	float _zn{ 0.f };
	float _zf{ 0.f };

	
	Frustum() {}
	Frustum(float fov, float ar, float zn, float zf);
	Frustum(const SMatrix& pm);

	static std::array<SVec3, 8> extractCorners(const SMatrix& pMat);//, const SMatrix& cMat);
	
	void update(const SMatrix& vpMat);
	std::vector<float> calcSplitDistances(uint8_t n) const;
	std::vector<SMatrix> createCascadeProjMatrices(uint8_t n, const std::vector<float>& zees) const;

	inline static Frustum createFrustumWithPlanesOnly(SMatrix& vpMatrix)
	{
		Frustum f;
		f.extractPlanesFromMatrix(vpMatrix);
		return f;
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