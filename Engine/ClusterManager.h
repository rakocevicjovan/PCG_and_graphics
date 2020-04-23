#pragma once
#include "ComputeShader.h"
#include "SBuffer.h"
#include "Math.h"
#include "ColFuncs.h"
#include <array>
#include <immintrin.h>

#include "ThreadPool.h"

#define AVG_MAX_LIGHTS_PER_CLUSTER (128u)



struct ClusterNode
{
	SVec4 _min;
	SVec4 _max;

	ClusterNode(SVec3 min, SVec3 max) : _min(min), _max(max)
	{
		_min.w = _max.w = 1.;
	}
};



struct LightIndexListItem
{
	uint16_t _index;
	uint16_t _count;		// Could likely get away with 8 here but it aligns the struct to 4 bytes anyways, it's compact enough

	LightIndexListItem(uint16_t index, uint16_t count) : _index(index), _count(count) { sizeof(LightIndexListItem); }
};



class ClusterManager
{
private:

	std::array<UINT, 3> _gridDims;
	std::array<UINT, 3> _invGrid;
	UINT _gridSize;
	
	/*
	// For GPU based implementation
	ComputeShader _csCuller;
	SBuffer _lightList;	// For simplicity just do point lights for now
	SBuffer _lightIndexList;
	SBuffer _clusterGrid;
	*/

	// CPU version, separate to a different class later and make both
	std::vector<uint16_t> _lightList;
	std::vector<LightIndexListItem> _offsetList;	// Contains offsets AND counts!
	std::vector<ClusterNode> _grid;

	std::vector<SVec4> _planes;

	//ThreadPool<const PLight&, SMatrix, SMatrix, float, float, float, float, SVec4> _threadPool;

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

		_invGrid[0] = 1.f / gridDims[0];
		_invGrid[1] = 1.f / gridDims[1];
		_invGrid[2] = 1.f / gridDims[2];

		/*
		// For GPU based implementation
		_csCuller.createFromFile(device, L"Culler.hlsl");
		_lightList = SBuffer(device, sizeof(lightSize), maxLights);
		_lightIndexList = SBuffer(device, sizeof(float) * 2u, _gridSize);
		*/

		_grid.reserve(_gridSize);

		_offsetList.resize(_gridSize);

		_lightList.resize(AVG_MAX_LIGHTS_PER_CLUSTER * _gridSize);
	}



	void assignLights(const std::vector<PLight>& pLights, const Camera& cam)
	{
		// buildGrid() exists to create explicitly defined bounds of each froxel approximated as a bounding AABB
		// However, culling one by one like that seems unnecessarily expensive! We can do better!
		// Cull once for each plane subdividing the frustum, reducing the cull count from (x * y * z) to (x + y + z)
		// Store min and max intersected plane indices, and compare indices when assigning lights per cluster!

		SMatrix v = cam.GetViewMatrix();
		SMatrix p = cam.GetProjectionMatrix();

		float p33 = p._33;
		float p34 = p._34;

		float zn = cam._frustum._zn;
		float zf = cam._frustum._zf;

		SVec4 invGridVec4(_invGrid[0], _invGrid[1], _invGrid[0], _invGrid[1]);

		// Convert all lights into clip space
		for (int i = 0; i < pLights.size(); ++i)
		{
			const PLight& pl = pLights[i];

			SVec3 ws_lightPos(pLights[i]._posRange);
			float lightRadius = pl._posRange.w;

			// Get light bounds in view space, radius stays the same
			SVec4 vs_lightPosRange = Math::fromVec3(SVec3::TransformNormal(ws_lightPos, v), lightRadius);	// x, y
			SVec2 minMax = SVec2(vs_lightPosRange.z) + SVec2(-lightRadius, lightRadius);					// z

			// Get light bounds in clip space
			SVec4 rect = getProjectedRectangle(vs_lightPosRange, zn, zf, p);								// x, y
			SVec2 clipZMinMax((minMax.x * p33 + p34) / minMax.x, (minMax.y * p33 + p34) / minMax.y);		// z

			std::array<uint16_t, 6> indices = getLightMinMaxIndices(rect, invGridVec4, clipZMinMax);

			for (int x = indices[0]; x < indices[2]; ++x)
			{
				for (int y = indices[1]; y < indices[3]; ++y)
				{
					for (int z = indices[4]; z < indices[5]; ++z)
					{

					}
				}
			}
		}

		/* Multithreaded version, should get to it eventually

		// Use this at one point, for now it's ok without
		 _threadPool.addJob(std::bind(&ClusterManager::getLightBoundsInClipSpace, ...));
		 getLightBoundsInClipSpace(pLights[i], v, p, cam._frustum._zn, cam._frustum._zf, p33, p34, invGridVec4);

		// Pass most of it by value (array in chunks!) or threads will slow down a lot!
		void getLightBoundsInClipSpace(const PLight& pl, SMatrix v, SMatrix p, float zn, float zf, float p33, float p34, SVec4 invGridVec4) {}
		*/
	}


	// Get min/max indices of grid clusters
	inline std::array<uint16_t, 6> getLightMinMaxIndices(const SVec4& rect, const SVec4& invGridVec4, const SVec2& clipZMinMax)
	{
		// This is more to learn SSE a bit than a real speed up because its just a few muls anyways...
		__m128 rectSSE = _mm_set_ps(rect.x, rect.y, rect.z, rect.w);
		__m128 invGridSSE = _mm_set_ps(invGridVec4.x, invGridVec4.y, invGridVec4.z, invGridVec4.w);

		__m128 res = _mm_mul_ps(rectSSE, invGridSSE);

		SVec2 zi = clipZMinMax * _invGrid[2];
		
		return { res.m128_u32[0], res.m128_u32[1], res.m128_u32[2], res.m128_u32[3], zi.x, zi.y };
		
		// This returns floats so I'll rather try to use SSE at least for the SVec4
		//SVec4 xyi = rect * invGridVec4;
		//SVec2 zi = clipZMinMax * _invGrid[2];
		// return { xyi.x, xyi.y, xyi.z, xyi.w, zi.x, zi.y };

		// If the above really vectorizes (the SimpleMath part) then it should be faster than this
		//int uint16_t = rect.x * _invGrid[0];
		//int uint16_t = rect.y * _invGrid[1];
		//int uint16_t = rect.z * _invGrid[0];
		//int uint16_t = rect.w * _invGrid[1];
		//int uint16_t = clipMinMax.x * _invGrid[2];
		//int uint16_t = clipMinMax.y * _invGrid[2];
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
			plane.w = viewToProjectedDepth(zNear, zFar, zV);
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
			n = viewToProjectedDepth(zNear, zFar, nV);

			fV = zSliceToViewDepth(zNear, zFar, zSlice + 1u, _gridDims[2]);	// Get required linear depth according to slice
			f = viewToProjectedDepth(zNear, zFar, fV);							// Transform it into projected Z

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

		SVec4 vsDirection = clip2view(SVec4(dirX, dirY, 0.f, 1.f), invProj);
		SRay viewRay(SVec3(0.f), SVec3(&vsDirection.x));

		SVec3 temp;
		Col::RayPlaneIntersection(viewRay, zPlane, temp);
		return temp;
	}



	inline SVec4 clip2view(SVec4 clip, SMatrix invProj)
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



	inline float viewToProjectedDepth(float zNear, float zFar, float viewDepth)
	{
		return (zFar * (viewDepth - zNear)) / ((zFar - zNear) * viewDepth);
	}


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