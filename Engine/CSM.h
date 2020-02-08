#pragma once
#include "Camera.h"
#include <d3d11.h>

class CSM
{
	Frustum _frustum;
	//std::vector<Frustum> _frusta;
	
	ID3D11Texture2D* _shadowMap;



	void init()
	{

	}

	void prepareShadowPass(const SMatrix& lightViewMatrix)
	{
		std::vector<SMatrix> pms = _frustum.createCascadeProjMatrices(3);

		for (int i = 0; i < pms.size(); ++i)
		{
			SMatrix vpm = lightViewMatrix * pms[i];
			
			std::array<SVec3, 8> corners = _frustum.extractCorners(vpm);


		}
	}

};