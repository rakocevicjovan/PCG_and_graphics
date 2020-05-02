#pragma once
#include "ComputeShader.h"
#include "SBuffer.h"
#include <array>

class ClusterManagerGPU
{
	const std::array<UINT, 3> _gridDims;
	const UINT _gridSize;

	ComputeShader _csCuller;
	SBuffer _lightList;
	SBuffer _lightIndexListSB;
	SBuffer _clusterGrid;


	ClusterManagerGPU(ID3D11Device* device, UINT maxLights, UINT lightByteSize, std::array<UINT, 3> gridDims)
		: _gridDims(gridDims), _gridSize(gridDims[0] * gridDims[1] * gridDims[2])
	{
		// Wrote this when I knew nothing about anything, so review before continuing...
		//_csCuller.createFromFile(device, L"Culler.hlsl");
		//_lightList = SBuffer(device, lightByteSize, maxLights);
		//_lightIndexListSB = SBuffer(device, sizeof(float) * 2u, _gridSize);
	}
};