#pragma once
#include "ComputeShader.h"
#include "SBuffer.h"
#include "Math.h"
#include "ColFuncs.h"
#include <array>

class ClusterManager
{
private:
	
	ComputeShader _csCuller;
	
	SBuffer _lightList;	// For simplicity just do point lights for now
	SBuffer _lightIndexList;
	SBuffer _clusterGrid;

	std::array<UINT, 3> _gridDims;
	UINT _gridSize;

public:

	// Not sure about any of this yet, work in progress...
	ClusterManager(
		ID3D11Device* device, std::array<UINT, 3> gridDims, 
		uint16_t maxLights, uint8_t lightSize) 
		: _gridDims(gridDims)
	{
		_gridSize = gridDims[0] * gridDims[1] * gridDims[2];

		_csCuller.createFromFile(device, L"Culler.hlsl");



		_lightList = SBuffer(device, sizeof(lightSize), maxLights);
		_lightIndexList = SBuffer(device, sizeof(float) * 2u, _gridSize);
	}



	void buildClusterGrid(float zNear)
	{
		// Dimensions of a single cell in a [-1, 1] span for x and y axes, and [0, 1] span for z axis (like DX NDC)
		float w = 2. / _gridDims[0];
		float h = 2. / _gridDims[1];
		float sliceThickness = 1. / _gridDims[2];

		std::vector<AABB> _nodes;
		_nodes.resize(_gridSize);

		SRay viewRay;
		viewRay.position = SVec3::Zero;

		SPlane localNear(SVec3(0, 0, 1), 0);
		SPlane localFar(SVec3(0, 0, 1), 0);

		SVec3 min, max;
		float xL, xR, yB, yT;

		for (int zSlice = 0; zSlice < _gridDims[2]; ++zSlice)
		{
			localNear.w = zNear + zSlice * sliceThickness;	// Depth, linear for now, could change it
			localFar.w = zNear + (zSlice + 1.) * sliceThickness;

			for (int i = 0; i < _gridDims[0]; ++i)
			{
				xL = i * w - 1.f;
				xR = xL + w;

				for (int j = 0; j < _gridDims[1]; ++j)
				{
					yB = j * h - 1.f;
					yT = yB + h;

					viewRay.direction = Math::getNormalizedVec3(SVec3(xL, yB, zNear));
					Col::RayPlaneIntersection(viewRay, localNear, min);

					viewRay.direction = Math::getNormalizedVec3(SVec3(xR, yT, zNear));
					Col::RayPlaneIntersection(viewRay, localFar, max);

					_nodes.push_back({min, max});
				}
			}
		}
	}



	void populateClusterGrid()
	{

	}


};