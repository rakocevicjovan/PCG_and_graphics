#pragma once
#include "Camera.h"
#include <d3d11.h>

class CSM
{
	std::vector<Frustum> _lightFrusta;

	UINT _edgeLength;
	
	ID3D11Texture2D* _shadowMap;

public:

	void init()
	{
		// initialize n textures for shadow mapping... could be depth stencil only for starters, but transparent objects...
	}



	// Expects input frustum corners to be in world space
	// I always admire my naming abilities... but it is what it says it is :V 
	void createFrustumBoundingFrustum(const std::array<SVec3, 8>& corners, const SMatrix& lvpMat)
	{
		float minX, maxX;
		float minY, maxY;
		float minZ, maxZ;

		std::array<SVec3, 8> lClipSpaceCorners;

		// Transform the world space positions of frustum coordinates into light's clip space
		for (int i = 0; i < corners.size(); ++i)
		{
			SVec3 pos = SVec3::Transform(corners[i], lvpMat);
			lClipSpaceCorners[i] = pos;

			if (pos.x < minX) minX = pos.x;
			if (pos.x > maxX) maxX = pos.x;

			if (pos.y < minY) minY = pos.y;
			if (pos.y > minY) minY = pos.y;

			if (pos.z < minZ) minZ = pos.z;
			if (pos.z > minZ) minZ = pos.z;
		}
		
		float Sx = 2.f / (maxX - minX);
		float Sy = 2.f / (maxY - minY);

		float Ox = -0.5 * (maxX + minX) * Sx;
		float Oy = -0.5 * (minY + minY) * Sy;

		SMatrix cropMatrix(
			Sx, 0., 0., Ox,
			0., Sy, 0., Oy,
			0,  0,  1., 0.,
			0., 0., 0., 1.);

		SMatrix Pz = DirectX::XMMatrixOrthographicLH(_edgeLength, _edgeLength, minZ, maxZ);

		SMatrix projMatrix = cropMatrix * Pz;

	}



	void prepareShadowPass(const Camera& cam, const SMatrix& lightViewMatrix, const SMatrix& lightProjMatrix)
	{
		std::vector<SMatrix> pms = cam._frustum.createCascadeProjMatrices(3);

		SMatrix lvpMat = lightViewMatrix * lightProjMatrix;

		for (int i = 0; i < pms.size(); ++i)
		{
			// In order to obtain the corners in world space, combine camera's view matrix and subdivision projection matrices
			SMatrix vpm = cam.GetViewMatrix() * pms[i];
			
			// Obtain the corners
			std::array<SVec3, 8> corners = Frustum::extractCorners(vpm);

			createFrustumBoundingFrustum(corners, lvpMat);

		}
	}



};