#pragma once
#include "ComputeShader.h"
#include "SBuffer.h"
#include "Math.h"
#include "ColFuncs.h"
#include "PoolAllocator.h"
#include <array>
#include <immintrin.h>

#include "ThreadPool.h"

#define AVG_MAX_LIGHTS_PER_CLUSTER (128u)



typedef std::array<uint8_t, 6> LightBounds;



//std::vector<ClusterNode> _grid;	// Explicit grid version
struct ClusterNode
{
	SVec4 _min;
	SVec4 _max;

	ClusterNode(SVec3 min, SVec3 max) : _min(min), _max(max)
	{
		_min.w = _max.w = 1.f;
	}
};



struct LightIndexListItem
{
	uint16_t _index;
	uint16_t _count;		// Could likely get away with 8 here but it aligns the struct to 4 bytes anyways, it's compact enough

	LightIndexListItem(uint16_t index, uint16_t count) : _index(index), _count(count) { sizeof(LightIndexListItem); }

	LightIndexListItem() : _index(0u), _count(0u) {}
};




class ClusterManager
{
private:

	std::array<UINT, 3> _gridDims;
	UINT _gridSize;

	
	// CPU version, separate to a different class later and make both
	std::vector<uint16_t> _lightIndexList;
	std::vector<LightIndexListItem> _offsetGrid;	// Contains offsets AND counts!

	// Helpers
	std::vector<SVec4> _planes;
	std::vector<LightBounds> _lightBounds;

	//PoolAllocator<uint16_t> _indexPool;
	//ThreadPool<const PLight&, SMatrix, SMatrix, float, float, float, float, SVec4> _threadPool;


	/*
	// For GPU based implementation
	ComputeShader _csCuller;
	SBuffer _lightList;
	SBuffer _lightIndexListSB;
	SBuffer _clusterGrid;
	/**/


public:


	// Not sure about any of this yet, work in progress...
	ClusterManager(
		ID3D11Device* device, 
		std::array<UINT, 3> gridDims,
		uint16_t maxLights
	) 
		: _gridDims(gridDims)
	{
		_gridSize = gridDims[0] * gridDims[1] * gridDims[2];
		_offsetGrid.resize(_gridSize);
		_lightIndexList.reserve(AVG_MAX_LIGHTS_PER_CLUSTER * _gridSize);


		/*
		// For GPU based implementation
		//_csCuller.createFromFile(device, L"Culler.hlsl");
		//_lightList = SBuffer(device, sizeof(lightSize), maxLights);
		//_lightIndexListSB = SBuffer(device, sizeof(float) * 2u, _gridSize);
		/**/
	}



	void assignLights(const std::vector<PLight>& pLights, const Camera& cam)
	{
		// buildGrid() exists to create explicitly defined bounds of each froxel approximated as a bounding AABB
		// However, culling one by one like that seems unnecessarily expensive! We can do better!
		
		// Option 1:
		// Cull once for each plane subdividing the frustum, reducing the cull count from (x * y * z) to (x + y + z)
		// Store min and max intersected plane indices, and compare indices when assigning lights per cluster!

		// Option 2:
		// No collision culling, convert lights to clip space and get bounds

		_lightIndexList.clear();	// From previous frame

		SMatrix v = cam.GetViewMatrix();
		SMatrix p = cam.GetProjectionMatrix();

		float p33 = p._33;
		float p43 = p._43;

		float zn = cam._frustum._zn;
		float zf = cam._frustum._zf;


		// Used for binning
		UINT zOffset = 0u;
		UINT yOffset = 0u;

		UINT sliceSize = _gridDims[0] * _gridDims[1];
		UINT rowSize = _gridDims[0];


		// Convert all lights into clip space
		for (int i = 0; i < pLights.size(); ++i)
		{
			const PLight& pl = pLights[i];

			SVec3 ws_lightPos(pLights[i]._posRange);
			float lightRadius = pl._posRange.w;

			// Get light bounds in view space, radius stays the same
			SVec4 vs_lightPosRange = Math::fromVec3(SVec3::TransformNormal(ws_lightPos, v), lightRadius);	// x, y
			SVec2 viewZMinMax = SVec2(vs_lightPosRange.z) + SVec2(-lightRadius, lightRadius);				// z

			// Convert XY light bounds to clip space, Z is calculated from view anyways
			SVec4 rect = getProjectedRectangle(vs_lightPosRange, zn, zf, p);

			LightBounds indexSpan = getLightMinMaxIndices(rect, viewZMinMax, zn, zf);
			_lightBounds.emplace_back(indexSpan);

			
			/*
			bool outsideFrustum =
				indexSpan[0] >= _gridDims[0] && indexSpan[1] < 0 &&
				indexSpan[2] >= _gridDims[1] && indexSpan[3] < 0 &&
				indexSpan[4] >= _gridDims[2] && indexSpan[5] < 0;

			if (outsideFrustum)
				continue;
			*/

			// First step of binning, increase counts per cluster
			for (int z = indexSpan[4]; z < indexSpan[5]; ++z)
			{
				zOffset = z * sliceSize;

				for (uint8_t y = indexSpan[1]; y < indexSpan[3]; ++y)
				{
					yOffset = y * rowSize;

					for (uint8_t x = indexSpan[0]; x < indexSpan[2]; ++x)
					{
						UINT cellIndex = zOffset + yOffset + x;	// Cell index in frustum's cluster grid, nbl to ftr

						_offsetGrid[cellIndex]._count++;
					}
				}
			}
		}


		// Second step of binning, determine offsets per cluster according to counts.
		UINT listStart = 0u;
		for (UINT i = 0u; i < _gridSize - 1; i++)
		{
			listStart += _offsetGrid[i]._count;
			_offsetGrid[i + 1]._index = listStart;
		}
		listStart += _offsetGrid.back()._count;


		if (_lightIndexList.size() < listStart)
		{
			_lightIndexList.reserve(listStart);
			_lightIndexList.resize(listStart);
		}
			


		// Third step of binning, insert lights to the contiguous list
		for (int i = 0; i < pLights.size(); i++)
		{
			for (int z = _lightBounds[i][4]; z < _lightBounds[i][5]; ++z)
			{
				zOffset = z * sliceSize;

				for (uint8_t y = _lightBounds[i][1]; y < _lightBounds[i][3]; ++y)
				{
					yOffset = y * rowSize;

					for (uint8_t x = _lightBounds[i][0]; x < _lightBounds[i][2]; ++x)
					{
						UINT cellIndex = zOffset + yOffset + x;

						int cellListStart = _offsetGrid[cellIndex]._index;
						int listOffset = _offsetGrid[cellIndex]._count--; // atomic on GPU
						_lightIndexList[cellListStart + listOffset - 1] = i;
					}
				}
			}
		}

		// Binning finished. A lot faster than my old version.

		/* Multithreaded version planned, should get to it eventually */
	}


	// Get min/max indices of grid clusters
	inline LightBounds getLightMinMaxIndices(const SVec4& rect, const SVec2& zMinMax, float zNear, float zFar)
	{
		// This returns floats so I'll rather try to use SSE at least for the SVec4
		SVec4 xyi = (rect + SVec4(1.f)) * SVec4(30., 17., 30., 17.) * 0.5f;

		uint8_t zMin = viewDepthToZSlice(zNear, zFar, zMinMax.x, _gridDims[2]);
		uint8_t zMax = viewDepthToZSlice(zNear, zFar, zMinMax.y, _gridDims[2]);

		return {
			static_cast<uint8_t>(xyi.x), static_cast<uint8_t>(xyi.y),
			static_cast<uint8_t>(xyi.z), static_cast<uint8_t>(xyi.w),
			zMin, zMax
		};

		// Learn SSE one day... THIS REVERSES THE ORDER OF STORED ELEMENTS BE CAREFUL!
		//__m128 rectSSE = _mm_load_ps(&rect.x);	// rect.y, rect.z, rect.w
		//__m128 invGridSSE = _mm_set_ps(30.f, 17.f, 30.f, 17.f);
		//__m128 res = _mm_mul_ps(rectSSE, invGridSSE);
		//__m128i intRes = _mm_cvttps_epi32(res);
		/*
		return
		{
			intRes.m128_u32[0], intRes.m128_u32[1],	// min indices
			intRes.m128_u32[2], intRes.m128_u32[3],	// max indices
			zMin, zMax
		};
		*/
	}



	void buildClipSpacePlanes(const Camera& cam)
	{
		_planes.reserve(_gridDims[0] + _gridDims[1] + _gridDims[2] + 3);	// Because it's a number of edge planes, not cells

		float zNear = cam._frustum._zn;
		float zFar = cam._frustum._zf;

		SMatrix invProj = cam.GetProjectionMatrix().Invert();

		// Slice widths
		float w = 2. / _gridDims[0];
		float h = 2. / _gridDims[1];

		SVec4 plane;
		

		plane = SVec4(0., 0., 1., 0.);

		for (int zSlice = 0; zSlice <= _gridDims[2]; ++zSlice)
		{
			float zV = zSliceToViewDepth(zNear, zFar, zSlice, _gridDims[2]);
			plane.w = zViewToZClip(zNear, zFar, zV);
			_planes.push_back(plane);
		}


		plane = SVec4(1., 0., 0., 0.);

		for (int i = 0; i <= _gridDims[0]; ++i)
		{
			plane.w = i * w - 1.f;
			_planes.push_back(plane);
		}


		plane = SVec4(0., 1., 0., 0.);

		for (int j = 0; j <= _gridDims[1]; ++j)
		{
			plane.w = j * h - 1.f;
			_planes.push_back(plane);
		}
	}



	void buildGrid(const Camera& cam)
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
			nV = zSliceToViewDepth(zNear, zFar, zSlice, _gridDims[2]);
			n = zViewToZClip(zNear, zFar, nV);

			fV = zSliceToViewDepth(zNear, zFar, zSlice + 1u, _gridDims[2]);	// Get required linear depth according to slice
			f = zViewToZClip(zNear, zFar, fV);							// Transform it into projected Z

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

#ifdef VIEW_SPACE_CLUSTERS			
					// All of this is just there to get to view space, which is only important IF we are culling in it
					// Otherwise, the corners of each AABB (frustum in clip space) are known already from simple math above

					SVec4 lbnView = unprojectPoint(SVec4(xL, yB, n, 1.), nV, invProj);
					SVec4 lbfView = unprojectPoint(SVec4(xL, yB, f, 1.), fV, invProj);

					SVec4 trnView = unprojectPoint(SVec4(xR, yT, n, 1.), nV, invProj);
					SVec4 trfView = unprojectPoint(SVec4(xR, yT, f, 1.), fV, invProj);

					min.x = min(lbnView.x, lbfView.x);
					min.y = min(lbnView.y, lbfView.y);

					max.x = max(trnView.x, trfView.x);
					max.y = max(trnView.y, trfView.y);

					_grid.emplace_back(min, max);

					// Alternative method examples, both cases ray points to zNear! But it intersects with further plane
					// lbnView = viewRayDepthSliceIntersection(SVec3(xL, yB, zNear), nV, invProj);
					// trfView = viewRayDepthSliceIntersection(SVec3(xR, yT, zNear), fV, invProj);

					// Tutorial author's method, same result as above, and same as my no intersection method
					//lbnView = viewRayDepthSliceIntersection(xL, yB, nV, invProj);
#endif
				}
			}
		}
	}



	// My preferred method, z project and unproject, in SVec4 w should be 1 (because of vectorized multiply)
	inline SVec4 unprojectPoint(SVec4 clipSpace, float w, const SMatrix& invProj)
	{
		return SVec4::Transform((clipSpace * w), invProj);
	}



	// My method, precalculate ray direction z and simply unproject the ray
	inline SVec3 viewRayDepthSliceIntersection(SVec3 rayDir, float vs_planeZ, const SMatrix& invProj)
	{
		SPlane zPlane(SVec3(0, 0, 1), -vs_planeZ);

		SRay viewRay(SVec3(0.f), rayDir);	// No normalization, just shoot the ray, seems to be correct
		viewRay.direction = SVec3::Transform(viewRay.direction, invProj);

		SVec3 temp;
		Col::RayPlaneIntersection(viewRay, zPlane, temp);
		return temp;
	}


	// From the tutorial, use z = 0 and w = 1, then convert ray direction to view space, gives same results
	inline SVec3 viewRayDepthSliceIntersection(float dirX, float dirY, float vs_planeZ, const SMatrix& invProj)
	{
		SPlane zPlane(SVec3(0, 0, 1), -vs_planeZ);

		SVec4 vsDirection = clipToView(SVec4(dirX, dirY, 0.f, 1.f), invProj);
		SRay viewRay(SVec3(0.f), SVec3(&vsDirection.x));

		SVec3 temp;
		Col::RayPlaneIntersection(viewRay, zPlane, temp);
		return temp;
	}



	inline SVec4 clipToView(SVec4 clip, SMatrix invProj)
	{
		SVec4 view = SVec4::Transform(clip, invProj);	// View space transform
		return (view / view.w);							// Perspective division
	}


	/* Taken from Doom presentation http://advances.realtimerendering.com/s2016/Siggraph2016_idTech6.pdf page 5/58 */
	inline float zSliceToViewDepth(float zNear, float zFar, uint8_t slice, uint8_t numSlices)
	{
		float exponent = static_cast<float>(slice) / numSlices;
		return zNear * pow((zFar / zNear), exponent);
	}


	inline uint8_t viewDepthToZSlice(float n, float f, float viewDepth, float Sz)
	{
		return log(viewDepth) * Sz / log(f / n) - Sz * log(n) / log(f / n);
	}


	inline float zViewToZClip(float zNear, float zFar, float viewDepth)
	{
		return (zFar * (viewDepth - zNear)) / ((zFar - zNear) * viewDepth);
	}


	inline float clipZToViewZ(float n, float f, float z)
	{
		return (n*f) / (f + (n - f) * z);
	}











	// Added so I don't get sued by intel when my engine inevitably becomes the ultimate engine in the eternity of the universe... /s

	// Copyright 2010 Intel Corporation
	// All Rights Reserved
	//
	// Permission is granted to use, copy, distribute and prepare derivative works of this
	// software for any purpose and without fee, provided, that the above copyright notice
	// and this statement appear in all copies.  Intel makes no representations about the
	// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
	// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
	// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
	// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
	// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
	// assume any responsibility for any errors which may appear in this software nor any
	// responsibility to update it.


	// Functions below are from intel's demo, but I derived the math to understand how it works. (29th june page, green notebook)
	// Four planes are created so that that each plane plane:
	// 1. Contains either x or y axes, passing through the view space origin (x = 0 or y = 0 in plane normal)
	// 2. Is tangential to the sphere that we are testing (mathematically, d = r where d = pNormal.Dot(sphereCenter))
	// By solving these equations, we get the planes that project to an axis aligned rectangle in clip space

	void updateClipRegionRoot(
		float nc,          // Tangent plane x or y normal coordinate (view space)
		float lc,          // Light x or y coordinate (view space)
		float lz,          // Light z coordinate (view space)
		float lightRadius,
		float cameraScale, // Project scale for coordinate (_11 or _22 for x/y respectively)
		float& clipMin,
		float& clipMax)
	{
		float nz = (lightRadius - nc * lc) / lz;
		float pz = (lc * lc + lz * lz - lightRadius * lightRadius) / (lz - (nz / nc) * lc);

		if (pz > 0.0f)
		{
			float c = -nz * cameraScale / nc;
			if (nc > 0.0f)
				clipMin = max(clipMin, c);		// Left side boundary, (x or y >= -1.)
			else
				clipMax = min(clipMax, c);		// Right side boundary, (x or y <= 1.)
		}
	}



	void updateClipRegion(
		float lc,				// Light x or y coordinate (view space)
		float lz,				// Light z coordinate (view space)
		float lightRadius,
		float cameraScale,		// Projection scale for coordinate (_11 for x, or _22 for y)
		float& clipMin,
		float& clipMax)
	{
		float rSq = lightRadius * lightRadius;
		float lcSqPluslzSq = lc * lc + lz * lz;

		// Determinant, if det <= 0 light covers the entire screen this we leave the default values (-1, 1) for the rectangle
		float det = rSq * lc * lc - lcSqPluslzSq * (rSq - lz * lz);

		// Light does not cover the entire screen, solve the quadratic equation, update root (aka project)
		if (det > 0)
		{
			float a = lightRadius * lc;
			float b = sqrt(det);
			float invDenom = 1.f / lcSqPluslzSq;	//hopefully this saves us a division? maybe? probably optimized out anyways
			float nx0 = (a + b) * invDenom;
			float nx1 = (a - b) * invDenom;

			updateClipRegionRoot(nx0, lc, lz, lightRadius, cameraScale, clipMin, clipMax);
			updateClipRegionRoot(nx1, lc, lz, lightRadius, cameraScale, clipMin, clipMax);
		}
	}



	SVec4 getProjectedRectangle(SVec4 lightPosView, float zNear, float zFar, const SMatrix& proj)
	{
		float lightRadius = lightPosView.w;
		SVec4 clipRegion = SVec4(1, 1, 0, 0);

		// Fast way to cull lights that are far enough behind the camera to not reach the near plane
		if (lightPosView.z + lightRadius >= zNear)
		{
			SVec2 clipMin(-1.0f);
			SVec2 clipMax(1.0f);

			updateClipRegion(lightPosView.x, lightPosView.z, lightRadius, proj._11, clipMin.x, clipMax.x);
			updateClipRegion(lightPosView.y, lightPosView.z, lightRadius, proj._22, clipMin.y, clipMax.y);

			clipRegion = SVec4(clipMin.x, clipMin.y, clipMax.x, clipMax.y);
		}

		return clipRegion;
	}
};



/*

// Slow boye version...
// This is not possible (to my knowledge at least) to optimize on a cpu to 2 ms, even with multithreading (I only have 4 cores!)


UINT sliceSize = _gridDims[0] * _gridDims[1];
UINT rowSize = _gridDims[0];

for (int z = 0; z < _gridDims[2]; ++z)
{
	UINT zOffset = z * sliceSize;

	//float clipZMin = zViewToZClip(zn, zf, zSliceToViewDepth(zn, zf, z, _gridDims[2]));
	//float clipZMax = zViewToZClip(zn, zf, zSliceToViewDepth(zn, zf, z + 1, _gridDims[2]));

	for (uint8_t y = 0; y < _gridDims[1]; ++y)
	{
		UINT yOffset = y * rowSize;

		for (uint8_t x = 0; x < _gridDims[0]; ++x)
		{
			UINT cellIndex = zOffset + yOffset + x;	// Cell index in frustum's cluster grid, nbl to ftr

			uint16_t lightIndexListStart = _lightIndexList.size();

			for (uint16_t i = 0; i < _lightBounds.size(); ++i)
			{
				LightBounds cpl = _lightBounds[i];		//minX, minY, maxX, maxY

				// omit z for now, need to find a good way to get indices for it
				// (probs homebrew binary search in std::array, as set/map indirections would kill perf...)

				bool inX = (x > cpl[0] && x < cpl[2]);
				bool inY = (y > cpl[1] && y < cpl[3]);
				bool inZ = true;

				if (inX && inY && inZ)
				{
					_lightIndexList.push_back(i);
				}
			}

			// Update the cluster grid
			//uint16_t cellLightCount = _lightIndexList.size() - lightIndexListStart;
			//_offsetGrid[cellIndex] = { lightIndexListStart, cellLightCount };
		}
	}
}
*/