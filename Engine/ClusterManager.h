#pragma once
#include "Light.h"
#include "Camera.h"
#include "ClusteringMath.h"
#include "VitThreadPool.h"
#include "SBuffer.h"
#include <array>
#include <immintrin.h>
#include <atomic>



#define AVG_MAX_LIGHTS_PER_CLUSTER (64u)



typedef std::array<uint8_t, 6> LightBounds;



struct OffsetListItem
{
	OffsetListItem() : _index(0u), _count(0u) {}
	OffsetListItem(const OffsetListItem&) = delete;
	OffsetListItem(const OffsetListItem&& other) : _index(std::move(other._index)), _count(other._count.load()) {}

	OffsetListItem& operator=(const OffsetListItem&) = delete;

	uint16_t _index;
	std::atomic<uint16_t> _count;		// Could likely get away with 8 here but it aligns the struct to 4 bytes anyways, it's compact enough
};



class ClusterManager
{

public:

	ClusterManager(std::array<UINT, 3> gridDims, uint16_t maxLights, ID3D11Device* device);

	void assignLights(const std::vector<PLight>& pLights, const Camera& cam, ctpl::thread_pool& threadPool);
	void upload(ID3D11DeviceContext* context, const std::vector<PLight>& lights);


private:

	const std::array<UINT, 3> _gridDims;
	const UINT _gridSize;

	std::vector<uint16_t> _lightIndexList;
	std::vector<OffsetListItem> _offsetGrid;	// Contains offsets and counts
	std::vector<LightBounds> _lightBounds;		// Intermediate data for binning

	SBuffer _lightSB;
	SBuffer _indexSB;
	SBuffer _gridSB;

	ID3D11ShaderResourceView* _lightSRV;
	ID3D11ShaderResourceView* _indexSRV;
	ID3D11ShaderResourceView* _gridSRV;

	static LightBounds getLightBounds(const PLight& pLight, float zn, float zf, const SMatrix& v, const SMatrix& p,
		std::array<UINT, 3u> gridDims, float _sz_div_log_fdn, float log_n);
	
	static void processLightsMT(
		const std::vector<PLight>& pLights, std::vector<LightBounds>& lightBounds, std::vector<OffsetListItem>& grid, 
		UINT min, UINT max, std::array<UINT, 3u> gridDims, float zn, float zf, float sz_div_log_fdn, float log_n, 
		const SMatrix& v, const SMatrix& p);



	// Get min/max indices of grid clusters, slightly optimized to use precalculated log(x), supposedly SVec is SIMD already?
	static inline LightBounds getLightMinMaxIndices
	(const SVec4& rect, const SVec2& zMinMax, float zNear, float zFar, std::array<UINT, 3> gDims, float _sz_div_log_fdn, float _log_n)
	{
		// This returns floats so I'll rather try to use SSE at least for the SVec4
		SVec4 xyi = ((rect + SVec4(1.f)) * 0.5f) * SVec4(gDims[0], gDims[1], gDims[0], gDims[1]);

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

	static void updateClipRegionRoot(
		float nc,          // Tangent plane x or y normal coordinate (view space)
		float lc,          // Light x or y coordinate (view space)
		float lz,          // Light z coordinate (view space)
		float lightRadius,
		float cameraScale, // Project scale for coordinate (_11 or _22 for x/y respectively)
		float& clipMin,
		float& clipMax);



	static void updateClipRegion(
		float lc,				// Light x or y coordinate (view space)
		float lz,				// Light z coordinate (view space)
		float lightRadius,
		float cameraScale,		// Projection scale for coordinate (_11 for x, or _22 for y)
		float& clipMin,
		float& clipMax);



	static SVec4 getProjectedRectangle(SVec4 lightPosView, float zNear, float zFar, const SMatrix& proj);
};