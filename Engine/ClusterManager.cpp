#include "pch.h"
#include "ClusterManager.h"


ClusterManager::ClusterManager(std::array<UINT, 3> gridDims, uint16_t maxLights, ID3D11Device* device)
	: _gridDims(gridDims), _gridSize(gridDims[0] * gridDims[1] * gridDims[2])
{
	_offsetGrid.resize(_gridSize);
	_lightIndexList.reserve(AVG_MAX_LIGHTS_PER_CLUSTER * _gridSize);

	// For now only deal with point lights

	// max 32 kb on point lights
	_lightSB = SBuffer(device, sizeof(PLight), 1024);
	SBuffer::CreateSBufferSRV(device, _lightSB.getPtr(), 1024, _lightSRV);

	// 1020 kb max for indices, but 32 bit packed
	_indexSB = SBuffer(device, sizeof(uint32_t), _lightIndexList.capacity());
	SBuffer::CreateSBufferSRV(device, _indexSB.getPtr(), _lightIndexList.capacity(), _indexSRV);

	// ~32 kb for grid offset list
	_gridSB  = SBuffer(device, sizeof(OffsetListItem), _gridSize);
	SBuffer::CreateSBufferSRV(device, _gridSB.getPtr(), _gridSize, _gridSRV);
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
	SMatrix v = cam.getViewMatrix();
	SMatrix p = cam.getProjectionMatrix();

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
	for (UINT i = 0; i < nThreads; i++)
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
	

	// Do the exact same thing but on the main thread as well... should really just use the function though

	// Convert all lights into clip space and obtain their min/max cluster indices
	for (UINT i = nThreads * chunkSize; i < nLights; ++i)	// nThreads * chunkSize
	{
		const PLight& pl = pLights[i];

		LightBounds indexSpan = getLightBounds(pl, zn, zf, v, p, _gridDims, sz_div_log_fdn, log_n);

		_lightBounds[i] = indexSpan;

		// First step of binning, increase counts per cluster
		for (uint8_t z = indexSpan[4]; z <= indexSpan[5]; z++)
		{
			zOffset = z * sliceSize;

			for (uint8_t y = indexSpan[1]; y <= indexSpan[3]; y++)
			{
				yOffset = y * rowSize;

				for (uint8_t x = indexSpan[0]; x <= indexSpan[2]; x++)	// Cell index in frustum's cluster grid, nbl to ftr
				{
					_offsetGrid[zOffset + yOffset + x]._count.fetch_add(1, std::memory_order_seq_cst);//_count++;
				}
			}
		}
	}

	// Make sure all threads have finished. The rest of the algorithm is not yet multithreaded so it's the same as it was
	for (UINT i = 0; i < nThreads; i++)
		futures[i].wait();


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
	
	for (UINT i = 0; i < nLights; i++)
	{
		for (uint8_t z = _lightBounds[i][4]; z <= _lightBounds[i][5]; z++)
		{
			zOffset = z * sliceSize;

			for (uint8_t y = _lightBounds[i][1]; y <= _lightBounds[i][3]; y++)
			{
				yOffset = y * rowSize;

				for (uint8_t x = _lightBounds[i][0]; x <= _lightBounds[i][2]; x++)
				{
					UINT cellIndex = zOffset + yOffset + x;

					uint32_t cellListStart = _offsetGrid[cellIndex]._index;
					uint32_t listOffset = --(_offsetGrid[cellIndex]._count);
					_lightIndexList[cellListStart + listOffset] = i;
				}
			}
		}
	}

	// Binning finished. A lot faster than my old version.
	// Multithreaded version in the works, not that much of an improvement so far with binning being basically random access...

}


void ClusterManager::upload(ID3D11DeviceContext* context, const std::vector<PLight>& lights)
{
	_lightSB.upload(context, lights.data(), lights.size() * sizeof(PLight));
	_indexSB.upload(context, _lightIndexList.data(), _lightIndexList.size() * sizeof(uint32_t));
	_gridSB.upload(context, _offsetGrid.data(), _offsetGrid.size() * sizeof(OffsetListItem));

	context->PSSetShaderResources(15, 1, &_lightSRV);
	context->PSSetShaderResources(16, 1, &_indexSRV);
	context->PSSetShaderResources(17, 1, &_gridSRV);
}


// This function purely reads the matrices so I can pass them by ref, copying the 3 big data structures is a no-no as well
void ClusterManager::processLightsMT(
	const std::vector<PLight>& pLights, std::vector<LightBounds>& lightBounds, std::vector<OffsetListItem>& grid,
	UINT mindex, UINT maxdex, std::array<UINT, 3u> gridDims, float zn, float zf, float sz_div_log_fdn, float log_n,
	const SMatrix& v, const SMatrix& p)
{
	UINT sliceSize = gridDims[0] * gridDims[1];
	UINT rowSize = gridDims[0];

	for (UINT i = mindex; i < maxdex; ++i)
	{
		const PLight& pl = pLights[i];

		LightBounds indexSpan = getLightBounds(pl, zn, zf, v, p, gridDims, sz_div_log_fdn, log_n);

		lightBounds[i] = indexSpan;

		// First step of binning, increase counts per cluster
		for (int z = indexSpan[4]; z <= indexSpan[5]; ++z)
		{
			UINT zOffset = z * sliceSize;

			for (uint8_t y = indexSpan[1]; y <= indexSpan[3]; ++y)
			{
				UINT yOffset = y * rowSize;

				for (uint8_t x = indexSpan[0]; x <= indexSpan[2]; ++x)	// Cell index in frustum's cluster grid, nbl to ftr
				{
					grid[zOffset + yOffset + x]._count.fetch_add(1, std::memory_order_seq_cst);
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
	SVec4 vs_lightPosRange = Math::fromVec3(SVec3::Transform(ws_lightPos, v), lightRadius);
	SVec2 viewZMinMax = SVec2(vs_lightPosRange.z) + SVec2(-lightRadius, lightRadius);

	// Convert XY light bounds to clip space, clamps to [-1, 1] Z is calculated from view
	SVec4 rect = getProjectedRectangle(vs_lightPosRange, zn, zf, p);

	LightBounds res = getLightMinMaxIndices(rect, viewZMinMax, zn, zf, gridDims, sz_div_log_fdn, log_n);
	return res;
}


LightBounds ClusterManager::getLightMinMaxIndices(const SVec4& rect, const SVec2& zMinMax, float zNear, float zFar, std::array<UINT, 3> gDims, float _sz_div_log_fdn, float _log_n)
{
	// This returns floats so I'll rather try to use SSE at least for the SVec4
	SVec4 xyi = rect + SVec4(1.f);	// -1, 1 to 0, 2
	xyi *= 0.5f;	//0, 2 to 0, 1
	xyi *= SVec4(gDims[0], gDims[1], gDims[0], gDims[1]);	//0, 1 to 0, maxX/Y

	//uint8_t zMin = viewDepthToZSlice(zNear, zFar, zMinMax.x, gDims[2]);
	//uint8_t zMax = viewDepthToZSlice(zNear, zFar, zMinMax.y, gDims[2]);

	uint8_t zMin = viewDepthToZSliceOpt(_sz_div_log_fdn, _log_n, zMinMax.x);
	uint8_t zMax = viewDepthToZSliceOpt(_sz_div_log_fdn, _log_n, zMinMax.y);

	// This is fragile AF! @TODO inspect
	return
	{
		static_cast<uint8_t>(xyi.x),												// min x
		static_cast<uint8_t>(xyi.y),												// min y
		std::min(static_cast<uint8_t>(xyi.z), static_cast<uint8_t>(gDims[0] - 1u)),	// max x
		std::min(static_cast<uint8_t>(xyi.w), static_cast<uint8_t>(gDims[1] - 1u)),	// max y
		std::max(zMin, static_cast <uint8_t>(0)),									// min z
		std::min(zMax, static_cast<uint8_t>(gDims[2] - 1))							// max z
	};
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

	if (pz > 0.0)
	{
		float c = -nz * cameraScale / nc;
		if (nc > 0.0)
		{
			//clipMin = std::max(clipMin, c);		// Left side boundary, (x or y >= -1.)
			clipMin = Math::clamp(-1., 1., c);
		}	
		else
		{
			//clipMax = std::min(clipMax, c);		// Right side boundary, (x or y less than 1.)
			clipMax = Math::clamp(-1., 1., c);		// I suspect I need this because of false positives on frustum culling
		}
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

	// Determinant, if det less than 0 light covers the entire screen this we leave the default values (-1, 1) for the rectangle
	float det = rSq * lc * lc - lcSqPluslzSq * (rSq - lz * lz);

	// Light does not cover the entire screen, solve the quadratic equation, update root (aka project)
	if (det > 0.0)
	{
		float a = lightRadius * lc;
		float b = sqrt(det);
		float nx0 = (a + b) / lcSqPluslzSq;
		float nx1 = (a - b) / lcSqPluslzSq;

		updateClipRegionRoot(nx0, lc, lz, lightRadius, cameraScale, clipMin, clipMax);
		updateClipRegionRoot(nx1, lc, lz, lightRadius, cameraScale, clipMin, clipMax);
	}
}


SVec4 ClusterManager::getProjectedRectangle(SVec4 lightPosView, float zNear, float zFar, const SMatrix& proj)
{
	float lightRadius = lightPosView.w;
	SVec4 clipRegion = SVec4(1, 1, -1., -1.);

	// Fast way to cull lights that are far enough behind the camera to not reach the near plane
	if (lightPosView.z + lightRadius >= zNear)
	{
		SVec2 clipMin(-1.);	//(-0.999999f, -0.999999f);
		SVec2 clipMax(1.);	//(+0.999999f, +0.999999f);

		updateClipRegion(lightPosView.x, lightPosView.z, lightRadius, proj._11, clipMin.x, clipMax.x);
		updateClipRegion(lightPosView.y, lightPosView.z, lightRadius, proj._22, clipMin.y, clipMax.y);

		clipRegion = SVec4(clipMin.x, clipMin.y, clipMax.x, clipMax.y);
	}

	return clipRegion;
}