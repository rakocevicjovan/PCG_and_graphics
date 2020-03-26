#pragma once
#include "ComputeShader.h"
#include "SBuffer.h"
#include <array>

class ClusterManager
{
private:
	
	ComputeShader* _csCuller;
	
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

		_csCuller->createFromFile(device, L"Culler.hlsl");



		_lightList = SBuffer(device, sizeof(lightSize), maxLights);
		_lightIndexList = SBuffer(device, sizeof(float) * 2u, _gridSize);
	}



	void buildClusterGrid()
		
	{

	}



	void populateClusterGrid()
	{

	}


};