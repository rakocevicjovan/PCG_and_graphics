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
		std::array<UINT, 3> gridDims, 
		uint16_t maxLights, 
		uint8_t lightSize) 
		: _gridDims(gridDims)
	{
		_gridSize = gridDims[0] * gridDims[1] * gridDims[2];

		/*
		// For GPU based implementation
		_csCuller.createFromFile(device, L"Culler.hlsl");
		_lightList = SBuffer(device, sizeof(lightSize), maxLights);
		_lightIndexList = SBuffer(device, sizeof(float) * 2u, _gridSize);
		*/

		_grid.resize(_gridSize);
		_offsetList.resize(_gridSize);
		_lightList.resize(MAX_LIGHTS_PER_CLUSTER * _gridSize);
	}



	void buildClusterGrid(float zNear, float zFar, const SMatrix& invProj)
	{
		// Dimensions of a single cell in a [-1, 1] span for x and y axes, and [0, 1] span for z axis (like DX NDC)
		float w = 2. / _gridDims[0];
		float h = 2. / _gridDims[1];
		float sliceThickness = 1. / _gridDims[2];

		std::vector<ClusterNode> _nodes;
		_nodes.resize(_gridSize);

		SRay viewRay;
		viewRay.position = SVec3(0, 0, -zNear);

		SPlane localNear(SVec3(0, 0, 1), 0.f);
		SPlane localFar(SVec3(0, 0, 1), 0.f);	// This is adjusted in the loop, d = 0.f is intentional

		SVec3 min, max, temp1, temp2;
		float xL, xR, yB, yT;

		for (int zSlice = 0; zSlice < _gridDims[2]; ++zSlice)
		{
			// Depth, d is negative I think? Negate if so!
			localNear.w = localFar.w;
			localFar.w = getZSliceDepth(zNear, zFar, zSlice + 1u, _gridDims[2]);

			min.z = localNear.w;
			max.z = localFar.w;


			for (int i = 0; i < _gridDims[0]; ++i)
			{
				xL = i * w - 1.f;
				xR = xL + w;

				for (int j = 0; j < _gridDims[1]; ++j)
				{
					yB = j * h - 1.f;
					yT = yB + h;

					// Ray pointing to and intersecting:

					// bottom left
					viewRay.direction = Math::getNormalizedVec3(SVec3(xL, yB, zNear));
					// close plane
					Col::RayPlaneIntersection(viewRay, localNear, temp1);
					// far plane
					Col::RayPlaneIntersection(viewRay, localFar, temp2);

					min.x = min(temp1.x, temp2.x);
					min.y = min(temp1.y, temp2.y);

					// top right
					viewRay.direction = Math::getNormalizedVec3(SVec3(xR, yT, zNear));
					// close plane
					Col::RayPlaneIntersection(viewRay, localNear, temp1);
					// far plane
					Col::RayPlaneIntersection(viewRay, localFar, temp2);

					max.x = max(temp1.x, temp2.x);
					max.y = max(temp1.y, temp2.y);

					// AABBs to view space
					_nodes.emplace_back(min, max);

					SVec4::Transform(_nodes.back._min, invProj);
					SVec4::Transform(_nodes.back._max, invProj);
				}
			}
		}
	}



	void populateClusterGrid()
	{

	}



	/* Taken from Doom presentation http://advances.realtimerendering.com/s2016/Siggraph2016_idTech6.pdf page 5/58 */
	
	inline float getZSliceDepth(float zNear, float zFar, uint8_t slice, uint8_t numSlices)
	{
		return zNear * pow((zFar / zNear), (slice / numSlices));
	}
};