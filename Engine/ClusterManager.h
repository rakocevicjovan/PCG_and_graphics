#pragma once
#include "Light.h"
#include "Camera.h"
#include "ClusteringMath.h"
#include "PoolAllocator.h"
#include "ThreadPool.h"
#include <array>
#include <immintrin.h>
#include <thread>
#include <future>

#define AVG_MAX_LIGHTS_PER_CLUSTER (128u)



typedef std::array<uint8_t, 6> LightBounds;



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

	const std::array<UINT, 3> _gridDims;
	const UINT _gridSize;

	
	// CPU version, separate to a different class later and make both
	std::vector<uint16_t> _lightIndexList;
	std::vector<LightIndexListItem> _offsetGrid;	// Contains offsets and counts
	std::vector<LightBounds> _lightBounds;			// Intermediate data for binning
	//ThreadPool<const PLight&, SMatrix, SMatrix, float, float, float, float, SVec4> _threadPool;


	// Optimization (not that impactful tbh)
	float _sz_div_log_fdn;
	float _log_n;

public:

	ClusterManager(std::array<UINT, 3> gridDims, uint16_t maxLights);



	void assignLights(const std::vector<PLight>& pLights, const Camera& cam);



	// Get min/max indices of grid clusters
	inline LightBounds getLightMinMaxIndices(const SVec4& rect, const SVec2& zMinMax, float zNear, float zFar)
	{
		// This returns floats so I'll rather try to use SSE at least for the SVec4
		SVec4 xyi = ((rect + SVec4(1.f)) * 0.5f) * SVec4(_gridDims[0], _gridDims[1], _gridDims[0], _gridDims[1]);

		uint8_t zMin = viewDepthToZSliceOpt(_sz_div_log_fdn, _log_n, zMinMax.x);
		uint8_t zMax = viewDepthToZSliceOpt(_sz_div_log_fdn, _log_n, zMinMax.y);

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



	void multiTester()
	{
		// Obviously this will use a pool when testing is done, not spawn every frame!
		auto nThreads = std::thread::hardware_concurrency();

		UINT chunk = _gridSize / nThreads;

		std::vector<std::thread*> _threads;
		std::vector<std::promise<UINT>> _promises;
		std::vector<std::future<UINT>> _futures;

		_threads.resize(nThreads);
		_promises.resize(nThreads);
		_futures.reserve(nThreads);

		for (int i = 0; i < nThreads; i++)
		{
			UINT start = i * chunk;
			UINT end = start + chunk;	//min((i + 1u) * chunk, _gridSize - 1);

			_futures.push_back(_promises[i].get_future());
			//_threads[i] = new std::thread(&ClusterManager::prefixSum, this, start, end, std::move(_promises[i]));
			//_threads.emplace_back(&ClusterManager::prefixSum, i * chunk, (i + 1u) * chunk, _promises[i]);

		}

		for (int i = 0; i < nThreads; i++)
		{
			_threads[i]->join();
			UINT wat = _futures[i].get();

			char buffer[50];
			sprintf(buffer, "Result is: %d \n", wat);

			OutputDebugStringA(buffer);
		}
		
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
		float& clipMax);



	void updateClipRegion(
		float lc,				// Light x or y coordinate (view space)
		float lz,				// Light z coordinate (view space)
		float lightRadius,
		float cameraScale,		// Projection scale for coordinate (_11 for x, or _22 for y)
		float& clipMin,
		float& clipMax);



	SVec4 getProjectedRectangle(SVec4 lightPosView, float zNear, float zFar, const SMatrix& proj);
};