#pragma once
#include "ClusterManager.h"



ClusterManager::ClusterManager(std::array<UINT, 3> gridDims, uint16_t maxLights)
	: _gridDims(gridDims), _gridSize(gridDims[0] * gridDims[1] * gridDims[2])
{
	_offsetGrid.resize(_gridSize);
	_lightIndexList.reserve(AVG_MAX_LIGHTS_PER_CLUSTER * _gridSize);
}



void ClusterManager::assignLights(const std::vector<PLight>& pLights, const Camera& cam, ctpl::thread_pool& threadPool)
{
	// Lights passed in here should already be frustum culled
	// Functions used here could be used for culling too but they are slower.
	// It's simpler than changing binning or dealing with uint8s over/underflowing (and I want indices small)

	// Reset from previous frame
	_lightIndexList.clear();	
	
	UINT nLights = pLights.size();
	_lightBounds.resize(nLights);	//as opposed to reserve

	// Get some data out for easier access/naming
	SMatrix v = cam.GetViewMatrix();
	SMatrix p = cam.GetProjectionMatrix();

	float p33 = p._33;
	float p43 = p._43;

	float zn = cam._frustum._zn;
	float zf = cam._frustum._zf;

	// Do this once rather than per every light
	float sz_div_log_fdn = static_cast<float>(_gridDims[2]) / log(zf / zn);
	float log_n = log(zn);


	// Used for binning
	UINT zOffset = 0u;
	UINT yOffset = 0u;

	UINT sliceSize = _gridDims[0] * _gridDims[1];
	UINT rowSize = _gridDims[0];

	
	// Threading setup, distribute work to other threads ( 3 in my case, +1 main thread)
	UINT nThreads = threadPool.n_idle();
	std::vector<std::future<void>> futures(nThreads);
	UINT chunkSize = pLights.size() / (nThreads + 1u);

	// Launch nThreads 
	for (int i = 0; i < nThreads; i++)
	{
		UINT minOff = i * chunkSize;
		UINT maxOff = (i + 1) * chunkSize;

		// Lockless, uses atomic to count and threads do not touch same _lightBounds indices...
		futures[i] =
			threadPool.push(
				std::bind(processLightsMT, std::ref(pLights), std::ref(_lightBounds), std::ref(_offsetGrid),
					minOff, maxOff, _gridDims, zn, zf, sz_div_log_fdn, log_n,
					std::ref(v), std::ref(p))
			);
	}


	// Do the exact same thing but on the main thread as well

	// Convert all lights into clip space and obtain their min/max cluster indices
	for (int i = nThreads * chunkSize; i < nLights; ++i)
	{
		const PLight& pl = pLights[i];

		LightBounds indexSpan = getLightBounds(pl, zn, zf, v, p, _gridDims, sz_div_log_fdn, log_n);

		_lightBounds[i] = indexSpan;	// do not push or emplace here, use assignment

		// First step of binning, increase counts per cluster
		for (int z = indexSpan[4]; z < indexSpan[5]; ++z)
		{
			zOffset = z * sliceSize;

			for (uint8_t y = indexSpan[1]; y < indexSpan[3]; ++y)
			{
				yOffset = y * rowSize;

				for (uint8_t x = indexSpan[0]; x < indexSpan[2]; ++x)	// Cell index in frustum's cluster grid, nbl to ftr
				{
					_offsetGrid[zOffset + yOffset + x]._count++;
				}
			}
		}
	}

	// Make sure all threads have finished. The rest of the algorithm is not yet multithreaded so it's the same as it was
	for (int i = 0; i < nThreads; i++)
	{
		futures[i].wait();
	}

	// Second step of binning, determine offsets per cluster according to counts, as well as the lightIndexList size.
	// Slow in debug mode because of bounds checking but fast as items are only 4 bytes and contiguous
	UINT listStart = 0u;
	for (UINT i = 0u; i < _gridSize - 1u; i++)
	{
		listStart += _offsetGrid[i]._count;
		_offsetGrid[i + 1]._index = listStart;
	}
	listStart += _offsetGrid.back()._count;


	if (_lightIndexList.size() < listStart)
		_lightIndexList.resize(listStart);

	// Third step of binning, insert light indices to the contiguous list... 
	// This looks like a bunch of random memory access, however contention can be greatly reduced: 
	// 1. Split _lightBounds into nLights/nThreads chunks (each iterates minOffset to maxOffset)
	// 2. Use atomic decrement on ._count (operators ++ and -- are already overloaded on atomic)
	// 3. This makes _lightIndexList[cellListStart + listOffset] unique every time, but cache thrashing might cause issues
	
	for (int i = 0; i < nLights; i++)
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
					int listOffset = _offsetGrid[cellIndex]._count--;
					_lightIndexList[cellListStart + listOffset - 1] = i;
				}
			}
		}
	}

	// Binning finished. A lot faster than my old version.
	// Multithreaded version in the works, not that much of an improvement so far with binning being basically random access...
}



// This function purely reads the matrices so I can pass them by ref, copying big data structures is a no-no
void ClusterManager::processLightsMT(
	const std::vector<PLight>& pLights, std::vector<LightBounds>& lightBounds, std::vector<OffsetListItem>& grid,
	UINT min, UINT max, std::array<UINT, 3u> gridDims, float zn, float zf, float sz_div_log_fdn, float log_n,
	const SMatrix& v, const SMatrix& p)
{
	UINT sliceSize = gridDims[0] * gridDims[1];
	UINT rowSize = gridDims[0];

	for (int i = min; i < max; ++i)
	{
		const PLight& pl = pLights[i];

		LightBounds indexSpan = getLightBounds(pl, zn, zf, v, p, gridDims, sz_div_log_fdn, log_n);

		lightBounds[i] = indexSpan;

		// First step of binning, increase counts per cluster
		for (int z = indexSpan[4]; z < indexSpan[5]; ++z)
		{
			UINT zOffset = z * sliceSize;

			for (uint8_t y = indexSpan[1]; y < indexSpan[3]; ++y)
			{
				UINT yOffset = y * rowSize;

				for (uint8_t x = indexSpan[0]; x < indexSpan[2]; ++x)	// Cell index in frustum's cluster grid, nbl to ftr
				{
					grid[zOffset + yOffset + x]._count++;	//fetch_add(1, std::memory_order_relaxed);
				}
			}
		}
	}
}



LightBounds ClusterManager::getLightBounds(const PLight& pLight, float zn, float zf, const SMatrix& v, const SMatrix& p,
	std::array<UINT, 3u> gridDims, float sz_div_log_fdn, float log_n)
{
	SVec3 ws_lightPos(pLight._posRange);
	float lightRadius = pLight._posRange.w;

	// Get light bounds in view space, radius stays the same
	SVec4 vs_lightPosRange = Math::fromVec3(SVec3::TransformNormal(ws_lightPos, v), lightRadius);
	SVec2 viewZMinMax = SVec2(vs_lightPosRange.z) + SVec2(-lightRadius, lightRadius);

	// Convert XY light bounds to clip space, clamps to [-1, 1] Z is calculated from view
	SVec4 rect = getProjectedRectangle(vs_lightPosRange, zn, zf, p);

	return getLightMinMaxIndices(rect, viewZMinMax, zn, zf, gridDims, sz_div_log_fdn, log_n);
}



void ClusterManager::updateClipRegionRoot(
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



void ClusterManager::updateClipRegion(
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



SVec4 ClusterManager::getProjectedRectangle(SVec4 lightPosView, float zNear, float zFar, const SMatrix& proj)
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