#pragma once
#include "Camera.h"
#include <d3d11.h>

class CSM
{
	std::vector<Frustum> _lightFrusta;

	UINT _resolution;
	
	ID3D11Texture2D* _shadowMap;

public:

	void init()
	{
		// initialize n textures for shadow mapping... could be depth stencil only for starters, but transparent objects...
	}



	// Expects input frustum corners to be in world space
	SMatrix createLightProjectionMatrix(const std::array<SVec3, 8>& corners, const SMatrix& lvpMat) const
	{
		float minX, maxX;
		float minY, maxY;
		float minZ, maxZ;

		minX = minY = minZ = 999999.f;	//use limits...
		maxX = maxY = maxZ = -999999.f;

		std::array<SVec3, 8> lViewSpaceCorners;

		// Transform the world space positions of frustum coordinates into light's clip space
		for (int i = 0; i < corners.size(); ++i)
		{
			SVec3 pos = SVec3::Transform(corners[i], lvpMat);
			lViewSpaceCorners[i] = pos;

			if (pos.x < minX) minX = pos.x;
			if (pos.x > maxX) maxX = pos.x;

			if (pos.y < minY) minY = pos.y;
			if (pos.y > maxY) maxY = pos.y;

			if (pos.z < minZ) minZ = pos.z;
			if (pos.z > maxZ) maxZ = pos.z;
		}
		
		/*
		// Calculate the crop matrix, which should limit the orthographic matrix to only encompass the frustum
		float Sx = 2.f / (maxX - minX);
		float Sy = 2.f / (maxY - minY);

		float Ox = -0.5 * (maxX + minX) * Sx;
		float Oy = -0.5 * (minY + minY) * Sy;

		SMatrix cropMatrix(
			Sx, 0., 0., Ox,
			0., Sy, 0., Oy,
			0,  0,  1., 0.,
			0., 0., 0., 1.);

		// @TODO If done like this, only objects in the camera frustum will cast shadows, need to account for it
		SMatrix Pz = DirectX::XMMatrixOrthographicLH(128, 128, minZ, maxZ);	// @TODO convert numbers to resolution
		SMatrix projMatrix = Pz * cropMatrix;	// I think this is reversed...
		*/

		SMatrix Pz = DirectX::XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);

		return Pz;
	}



	std::vector<SMatrix> calcProjMats(const Camera& cam, const SMatrix& lightViewMatrix) const
	{
		std::vector<SMatrix> projMats;
		std::vector<SMatrix> camFrustumSubdivisionPMs = cam._frustum.createCascadeProjMatrices(3);

		SMatrix lvMat = lightViewMatrix;

		for (int i = 0; i < camFrustumSubdivisionPMs.size(); ++i)
		{
			// Obtain the corners in world space
			std::array<SVec3, 8> corners = Frustum::extractCorners(cam.GetViewMatrix() * camFrustumSubdivisionPMs[i]);	//, cam.GetCameraMatrix()
			projMats.push_back(createLightProjectionMatrix(corners, lvMat));	// Transform them to light space etc...
		}

		return projMats;
	}



	std::vector<Frustum> createShadowPassFrusta(const Camera& cam, const SMatrix& dirLightViewMatrix, const SMatrix dirLightCamMatrix)
	{
		std::vector<SMatrix> projMats = calcProjMats(cam, dirLightViewMatrix);
		
		std::vector<Frustum> frusta;
		frusta.reserve(projMats.size());

		for (int i = 0; i < projMats.size(); ++i)
		{
			//frustumRenderable._transform = projMats[i].Invert() * dirLightCamMatrix;
			//S_RANDY.render(frustumRenderable);

			frusta.emplace_back(projMats[i] * dirLightCamMatrix);
		}

		return frusta;
	}
};