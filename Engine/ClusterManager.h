#pragma once
#include "ComputeShader.h"
#include "SBuffer.h"
#include "Math.h"
#include "ColFuncs.h"
#include <array>

#define MAX_LIGHTS_PER_CLUSTER (128u)

struct ClusterNode
{
	SVec4 _min;
	SVec4 _max;

	ClusterNode(SVec3 min, SVec3 max) : _min(min), _max(max)
	{
		_min.w = 1.;
		_max.w = 1.;
	}
};

class ClusterManager
{
private:

	std::array<UINT, 3> _gridDims;
	UINT _gridSize;
	
	/*
	// For GPU based implementation
	ComputeShader _csCuller;
	SBuffer _lightList;	// For simplicity just do point lights for now
	SBuffer _lightIndexList;
	SBuffer _clusterGrid;
	*/

	// CPU version, separate to a different class later and make both
	std::vector<uint32_t> _offsetList;	// Contains offsets AND counts!
	std::vector<uint16_t> _lightList;
	std::vector<ClusterNode> _grid;

public:


	// Not sure about any of this yet, work in progress...
	ClusterManager(
		ID3D11Device* device, 
		std::array<UINT, 3> gridDims
		//uint16_t maxLights, 
		//uint8_t lightSize
	) 
		: _gridDims(gridDims)
	{
		_gridSize = gridDims[0] * gridDims[1] * gridDims[2];

		/*
		// For GPU based implementation
		_csCuller.createFromFile(device, L"Culler.hlsl");
		_lightList = SBuffer(device, sizeof(lightSize), maxLights);
		_lightIndexList = SBuffer(device, sizeof(float) * 2u, _gridSize);
		*/

		_grid.reserve(_gridSize);

		_offsetList.resize(_gridSize);

		_lightList.resize(MAX_LIGHTS_PER_CLUSTER * _gridSize);
	}



	void buildGrid(Camera cam)
	{
		float zNear = cam._frustum._zn;
		float zFar = cam._frustum._zf;
		SMatrix invProj = cam.GetProjectionMatrix().Invert();

		float w = 2. / _gridDims[0];
		float h = 2. / _gridDims[1];

		SVec3 min, max;
		float xL, xR, yB, yT;
		float n, nV, f, fV;

		for (int zSlice = 0; zSlice < _gridDims[2]; ++zSlice)
		{
			nV = getZSliceDepth(zNear, zFar, zSlice, _gridDims[2]);
			n = getProjectedDepth(zNear, zFar, nV);

			fV = getZSliceDepth(zNear, zFar, zSlice + 1u, _gridDims[2]);	// Get required linear depth according to slice
			f = getProjectedDepth(zNear, zFar, fV);							// Transform it into projected Z

			min.z = nV;
			max.z = fV;

			for (int i = 0; i < _gridDims[0]; ++i)
			{
				xL = i * w - 1.f;
				xR = xL + w;

				for (int j = 0; j < _gridDims[1]; ++j)
				{
					yB = j * h - 1.f;
					yT = yB + h;

					SVec4 lbnView = unprojectPoint(SVec4(xL, yB, n, nV), invProj);
					SVec4 lbfView = unprojectPoint(SVec4(xL, yB, f, fV), invProj);

					min.x = min(lbnView.x, lbfView.x);
					min.y = min(lbnView.y, lbfView.y);

					SVec4 trnView = unprojectPoint(SVec4(xR, yT, n, nV), invProj);
					SVec4 trfView = unprojectPoint(SVec4(xR, yT, f, fV), invProj);

					max.x = max(trnView.x, trfView.x);
					max.y = max(trnView.y, trfView.y);

					_grid.emplace_back(min, max);

					// Alternative method examples, both cases ray points to zNear! But it intersects with further plane
					// lbnView = viewRayDepthSliceIntersection(SVec3(xL, yB, zNear), nV, invProj);
					// trfView = viewRayDepthSliceIntersection(SVec3(xR, yT, zNear), fV, invProj);

					// Tutorial author's method, same result as above, and same as mine (no intersection method)
					//lbnView = viewRayDepthSliceIntersection(xL, yB, nV, invProj);
				}
			}
		}
	}



	// My method, z project and unproject
	inline SVec4 unprojectPoint(SVec4 clipSpace, const SMatrix& invProj)
	{
		clipSpace.x *= clipSpace.w;
		clipSpace.y *= clipSpace.w;
		clipSpace.z *= clipSpace.w;
		return SVec4::Transform(clipSpace, invProj);
	}



	inline SVec4 viewRayDepthSliceIntersection(SVec3 rayDir, float vs_planeZ, const SMatrix& invProj)
	{
		SPlane zPlane(SVec3(0, 0, 1), -vs_planeZ);
		SRay viewRay(SVec3(0.f), rayDir);	// No normalization, just shoot the ray, seems to be working well
		viewRay.direction = SVec3::Transform(viewRay.direction, invProj);

		SVec3 temp;
		Col::RayPlaneIntersection(viewRay, zPlane, temp);
		return temp;
	}



	inline SVec4 viewRayDepthSliceIntersection(float dirX, float dirY, float vs_planeZ, const SMatrix& invProj)
	{
		SPlane zPlane(SVec3(0, 0, 1), -vs_planeZ);
		SVec4 test = clip2view(SVec4(dirX, dirY, 0.f, 1.f), invProj);
		SRay viewRay(SVec3(0.f), SVec3(&test.x));

		SVec3 temp;
		Col::RayPlaneIntersection(viewRay, zPlane, temp);
		return temp;
	}



	inline SVec4 clip2view(SVec4 clip, SMatrix invProj)
	{
		SVec4 view = SVec4::Transform(clip, invProj);	// View space transform
		return (view / view.w);							// Perspective projection
	}



	/* Taken from Doom presentation http://advances.realtimerendering.com/s2016/Siggraph2016_idTech6.pdf page 5/58 */
	inline float getZSliceDepth(float zNear, float zFar, uint8_t slice, uint8_t numSlices)
	{
		float exponent = static_cast<float>(slice) / numSlices;
		return zNear * pow((zFar / zNear), exponent);
	}



	inline float getProjectedDepth(float zNear, float zFar, float fV)
	{
		return (zFar * (fV - zNear)) / ((zFar - zNear) * fV);
	}
};