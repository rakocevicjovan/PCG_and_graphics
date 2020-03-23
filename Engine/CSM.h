#pragma once
#include "Camera.h"
#include "Texture.h"
#include "ShaderCompiler.h"
#include "Renderable.h"


struct ShadowBufferData
{
	SMatrix lvpMatrices[3];
	SVec4 cascadeLimits;			//rudimentary but ok for now, I will hardly need more than 4 cascades anyways
};



class CSM
{
	uint8_t _nMaps;
	UINT _width, _height;
	
	ID3D11Texture2D* _shadowMapArray;
	std::vector<ID3D11DepthStencilView*> _dsvPtrs;
	std::vector<D3D11_VIEWPORT> _viewports;
	ID3D11ShaderResourceView* _shadowResView, *_debugResView;

	// I trust MJP but not myself. Not sure if I even need this but want to know it's purpose.
	ID3D11DepthStencilView* _readOnlyDSV;

	std::vector<SMatrix> _lvpMats;
	std::vector<float> _distances;
	ID3D11InputLayout* _inLay;

	ID3D11Buffer* _wmBuffer;
	ID3D11Buffer* _lvpBuffer;
	ID3D11Buffer* _shadowBuffer;

	ShadowBufferData _shBuffData;

	VertexShader* _vs;
	// Alternative... I will want to use the pixel shader later to deal with transparency, for now only the depth stencil buffer.

public:

	bool init(ID3D11Device* device, uint8_t nMaps, UINT width, UINT height, VertexShader* vs)
	{
		_vs = vs;

		// Set the required parameters
		_width = width;
		_height = height;
		_nMaps = nMaps;

		_dsvPtrs.reserve(nMaps);

		_lvpMats.resize(nMaps);
		_viewports.resize(nMaps);


		// Initialize buffers used by the csm shader

		auto lptBufferDesc = ShaderCompiler::createBufferDesc(sizeof(SMatrix));
		if (FAILED(device->CreateBuffer(&lptBufferDesc, NULL, &_lvpBuffer)))
		{
			OutputDebugStringA("Failed to create CSM light view projection matrix buffer. ");
			return false;
		}

		auto wmBufferDesc = ShaderCompiler::createBufferDesc(sizeof(SMatrix));
		if (FAILED(device->CreateBuffer(&wmBufferDesc, NULL, &_wmBuffer)))
		{
			OutputDebugStringA("Failed to create CSM world matrix buffer. ");
			return false;
		}

		auto shadowBufferDesc = ShaderCompiler::createBufferDesc(sizeof(ShadowBufferData));
		if (FAILED(device->CreateBuffer(&shadowBufferDesc, NULL, &_shadowBuffer)))
		{
			OutputDebugStringA("Failed to create CSM shadow buffer. ");
			return false;
		}


		// Initialize GPU resources for shadow mapping... could be depth stencil only for starters, but transparent objects...

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


		// for debugging
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc2;
		srvDesc2.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc2.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc2.Texture2D.MipLevels = 1u;
		srvDesc2.Texture2D.MostDetailedMip = 0u;

		if (FAILED(device->CreateShaderResourceView(_shadowMapArray, &srvDesc2, &_debugResView)))
		{
			OutputDebugStringA("Can't create debug shader resource view. (CSM) \n");
			return false;
		}

		return true;
	}



	// Expects input frustum corners to be in world space
	SMatrix createLightProjectionMatrix(const std::array<SVec3, 8>& corners, const SMatrix& lvMat, uint8_t vpIndex)
	{
		float minX, maxX;
		float minY, maxY;
		float minZ, maxZ;

		minX = minY = minZ = 999999.f;	//could use limits instead...
		maxX = maxY = maxZ = -999999.f;

		std::array<SVec3, 8> lViewSpaceCorners;

		// Transform the world space positions of frustum coordinates into light's clip space
		for (int i = 0; i < corners.size(); ++i)
		{
			SVec3 pos = SVec3::Transform(corners[i], lvMat);
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

		// @TODO these ended up being all the same... I thought it differed at first, move this to single viewport class member
		//_viewports[vpIndex] = { (FLOAT)0.f, (FLOAT)0.f, (FLOAT)(abs(maxX - minX)), (FLOAT)(abs(maxZ - minZ)), 0.f, 1.f };

		_viewports[vpIndex] = { (FLOAT)0.f, (FLOAT)0.f, (FLOAT)_width, (FLOAT)_height, 0.f, 1.f };

		return DirectX::XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);
	}



	std::vector<SMatrix> calcProjMats(const Camera& cam, const SMatrix& lightViewMatrix)
	{
		std::vector<SMatrix> projMats;
		projMats.reserve(_nMaps);

		_distances = cam._frustum.calcSplitDistances(_nMaps);
		_shBuffData.cascadeLimits = SVec4(_distances.data());	// Unsafe, maybe?
		std::vector<SMatrix> camFrustumSubdivisionPMs = cam._frustum.createCascadeProjMatrices(_nMaps, _distances);

		for (int i = 0; i < camFrustumSubdivisionPMs.size(); ++i)
		{
			// Obtain the corners in world space
			std::array<SVec3, 8> corners = Frustum::extractCorners(cam.GetViewMatrix() * camFrustumSubdivisionPMs[i]);	//, cam.GetCameraMatrix()
			projMats.push_back(createLightProjectionMatrix(corners, lightViewMatrix, i));	// Transform them to light space etc...
			_lvpMats[i] = lightViewMatrix * projMats.back();
		}

		return projMats;
	}


	// All of this below will probably be refactored to just return it's state and not set it directly...
	void beginShadowPassSequence(ID3D11DeviceContext* context)
	{
		context->VSSetShader(_vs->_vsPtr, nullptr, 0);
		context->PSSetShader(NULL, nullptr, 0);
		_inLay = _vs->_layout;
	}



	void beginShadowPassN(ID3D11DeviceContext* context, uint8_t n)
	{
		context->ClearDepthStencilView(_dsvPtrs[n], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		context->OMSetRenderTargets(0, nullptr, _dsvPtrs[n]);

		context->RSSetViewports(1, &(_viewports[n]));

		SMatrix lvpMatTranspose = _lvpMats[n].Transpose();
		_shBuffData.lvpMatrices[n] = lvpMatTranspose;
		CBuffer::updateWholeBuffer(context, _lvpBuffer, &lvpMatTranspose, sizeof(SMatrix));
		context->VSSetConstantBuffers(1, 1, &_lvpBuffer);
	}



	void drawToCurrentShadowPass(ID3D11DeviceContext* context, Renderable& r)
	{
		//r.updateBuffersAuto(context);
		//r.setBuffers(_deviceContext);

		//this is not flexible, it must use the above somehow in order to work properly @TODO
		SMatrix transformTranspose = r._transform.Transpose();
		CBuffer::updateWholeBuffer(context, _wmBuffer, &transformTranspose, sizeof(SMatrix));
		context->VSSetConstantBuffers(0, 1, &_wmBuffer);

		context->IASetPrimitiveTopology(r.mat->primitiveTopology);
		context->IASetInputLayout(_inLay);

		UINT stride = r.mesh->getStride();
		UINT offset = r.mesh->getOffset();

		context->IASetVertexBuffers(0, 1, r.mesh->_vertexBuffer.ptr(), &stride, &offset);
		context->IASetIndexBuffer(r.mesh->_indexBuffer.ptr(), DXGI_FORMAT_R32_UINT, 0);

		context->DrawIndexed(r.mesh->indexCount, 0, 0);
	}



	void drawToSceneWithCSM(ID3D11DeviceContext* context, Renderable& r)
	{
		context->VSSetShader(r.mat->getVS()->_vsPtr, nullptr, 0);
		context->PSSetShader(r.mat->getPS()->_psPtr, nullptr, 0);

		//r.mat->getPS()->updateBuffersAuto(context, r);
		//r.mat->getPS()->setBuffers(context);

		// Again, bad, needs to be more flexible... but put together the pieces for now
		SMatrix transformTranspose = r._transform.Transpose();
		CBuffer::updateWholeBuffer(context, r.mat->getVS()->_cbuffers[0]._cbPtr, &transformTranspose, sizeof(SMatrix));
		context->VSSetConstantBuffers(0, 1, &(r.mat->getVS()->_cbuffers[0]._cbPtr));

		// Bind usual textures
		r.mat->bindTextures(context);
		context->PSSetSamplers(0, 1, &r.mat->getPS()->_sState);

		// Bind shadow map array
		context->PSSetShaderResources(11, 1, &_shadowResView);

		context->IASetInputLayout(r.mat->getVS()->_layout);
		context->IASetPrimitiveTopology(r.mat->primitiveTopology);

		UINT stride = r.mesh->getStride();
		UINT offset = r.mesh->getOffset();

		context->IASetVertexBuffers(0, 1, r.mesh->_vertexBuffer.ptr(), &stride, &offset);
		context->IASetIndexBuffer(r.mesh->_indexBuffer.ptr(), DXGI_FORMAT_R32_UINT, 0);

		context->DrawIndexed(r.mesh->indexCount, 0, 0);

		// Unbind shadow map array
		ID3D11ShaderResourceView *const pSRV[1] = { NULL };
		context->PSSetShaderResources(11, 1, pSRV);
	}



	void uploadCSMBuffer(ID3D11DeviceContext* context, UINT slot)
	{
		CBuffer::updateWholeBuffer(context, _shadowBuffer, &_shBuffData, sizeof(_shBuffData));
		context->PSSetConstantBuffers(slot, 1, &_shadowBuffer);
	}



	void unbindTextureArray(ID3D11DeviceContext* context)
	{
		ID3D11ShaderResourceView *const pSRV[1] = { NULL };
		context->PSSetShaderResources(PS_CSM_TEXTURE_REGISTER, 1, pSRV);
	}



	uint8_t getNMaps() { return _nMaps; }
	ID3D11ShaderResourceView* const* getResView() { return &_shadowResView;}
	ID3D11ShaderResourceView* getDebugView() { return _debugResView; }
};



/* Can be an useful function but not needed right now
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
*/