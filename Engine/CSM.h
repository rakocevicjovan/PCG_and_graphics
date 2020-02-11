#pragma once
#include "Camera.h"
#include "Texture.h"
#include <d3d11.h>

class CSM
{
	uint8_t _nMaps;
	UINT _width, _height;

	std::vector<Frustum> _lightFrusta;
	
	ID3D11Texture2D* _shadowMapArray;
	std::vector<ID3D11DepthStencilView*> _dsvPtrs;
	ID3D11ShaderResourceView* _shadowResView;

	// I trust MJP but not myself. Not sure if I even need this but want to know it's purpose.
	ID3D11DepthStencilView* _readOnlyDSV;		

	// Alternative... I will want to use the pixel shader later to deal with transparency, for now only the depth stencil buffer.
	//std::vector<ID3D11Texture2D*> _shadowMaps;
	//std::vector<ID3D11ShaderResourceView*> _shadowResViews;
	//std::vector<ID3D11RenderTargetView*> _shadowRTVs;

public:

	bool init(ID3D11Device* device, uint8_t nMaps, UINT width, UINT height)
	{
		_width = width;
		_height = height;
		_nMaps = nMaps;

		_dsvPtrs.reserve(nMaps);


		// initialize n textures for shadow mapping... could be depth stencil only for starters, but transparent objects...

		//DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT;
		//uint32_t bindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;


		D3D11_TEXTURE2D_DESC texDesc = Texture::create2DTexDesc(
			_width, _height, DXGI_FORMAT_R32_TYPELESS, D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL,	// | D3D11_BIND_RENDER_TARGET not necessary for ds I think
			0u, 0u, 1u, nMaps);

		// Create the base texture array object to be used through the subsequently created views, stores depth data per CSM frustum
		if (FAILED(device->CreateTexture2D(&texDesc, 0, &_shadowMapArray)))
		{
			OutputDebugStringA("Failed to create 2D texture. (CSM) \n");
			return false;
		}


		// Create depth stencil views used for writing during the creation of shadow maps
		for (uint8_t i = 0; i < nMaps; ++i)
		{
			ID3D11DepthStencilView* dsvPtr;
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;

			ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;	// Possibly wrong, will see
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDesc.Texture2DArray.ArraySize = 1;
			dsvDesc.Texture2DArray.FirstArraySlice = i;
			dsvDesc.Texture2DArray.MipSlice = 0;
			dsvDesc.Flags = 0;

			if (FAILED(device->CreateDepthStencilView(_shadowMapArray, &dsvDesc, &dsvPtr)))
			{
				OutputDebugStringA("Failed to create depth stencil view. (CSM) \n");
				return false;
			}
			_dsvPtrs.push_back(dsvPtr);

			if (i == 0)
			{
				// Also create a read-only DSV
				dsvDesc.Flags = D3D11_DSV_READ_ONLY_DEPTH;

				/* Don't think I ever use stencil for these so... that's that.
				if (format == DXGI_FORMAT_D24_UNORM_S8_UINT || format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
					dsvDesc.Flags |= D3D11_DSV_READ_ONLY_STENCIL;*/

				if (FAILED(device->CreateDepthStencilView(_shadowMapArray, &dsvDesc, &_readOnlyDSV)))
				{
					OutputDebugStringA("Failed to create read only depth stencil view (CSM). \n");
					return false;
				}
				dsvDesc.Flags = 0;
			}
		}


		// To read from texture during scene draw
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.ArraySize = nMaps;
		srvDesc.Texture2DArray.FirstArraySlice = 0u;
		srvDesc.Texture2DArray.MipLevels = 1u;
		srvDesc.Texture2DArray.MostDetailedMip = 0u;

		if (FAILED(device->CreateShaderResourceView(_shadowMapArray, &srvDesc, &_shadowResView)))
		{
			OutputDebugStringA("Can't create shader resource view. (CSM) \n");
			return false;
		}

		return true;
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