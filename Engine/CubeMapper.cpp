#include "pch.h"
#include "CubeMapper.h"
#include "Camera.h"
#include "CubeMap.h"


CubeMapper::CubeMapper(unsigned int edgeLength) : _edgeLength(edgeLength) {}



CubeMapper::~CubeMapper() {}



void CubeMapper::init(ID3D11Device* device)
{
	// Create a texture (and a description) for the cube mapper, using misc_texturecube and 4 8-bit channels
	D3D11_TEXTURE2D_DESC texDesc = CubeMap::createCubeMapDesc(_edgeLength, true, DXGI_FORMAT_R32G32B32A32_FLOAT);
	CubeMap::CreateCubeMap(device, texDesc, _texPtr);


	// Create ONE resource view as a texturecube view (this handles six faces internally, so one is enough)
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	srvd.Format = texDesc.Format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvd.TextureCube.MipLevels = texDesc.MipLevels;
	srvd.TextureCube.MostDetailedMip = 0;

	if (FAILED(device->CreateShaderResourceView(_texPtr, &srvd, &_shResView)))
	{
		OutputDebugStringA("Can't create shader resource view. \n");
		exit(521);
	}

	// Create SIX render target views, each render target view being a slice in an array (this is sketchy, @TODO)
	D3D11_RENDER_TARGET_VIEW_DESC rtvd;
	rtvd.Format = texDesc.Format;
	rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvd.Texture2D.MipSlice = 0;
	rtvd.Texture2DArray.ArraySize = 1;

	for (int i = 0; i < 6; i++)
	{
		// Indicates that the i-th texture of the _texPtr cubemap will be rendered to using this view (cubemap is a 2d array internally)
		rtvd.Texture2DArray.FirstArraySlice = i;

		if (FAILED(device->CreateRenderTargetView(_texPtr, &rtvd, &(_renderTargetViews[i]))))
		{
			OutputDebugStringA("Failed to create 6 render target views. \n");
			exit(522);
		}
	}


	// Create a depth stencil texture, same size as the original
	D3D11_TEXTURE2D_DESC depthTexDesc = {};
	depthTexDesc.Width = _edgeLength;
	depthTexDesc.Height = _edgeLength;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.SampleDesc = { 1, 0 };
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.MiscFlags = 0;

	if (FAILED(device->CreateTexture2D(&depthTexDesc, 0, &_depthTexPtr)))
	{
		OutputDebugStringA("Can't create cube map depth texture. \n");
		exit(523);
	}

	// Create a depth stencil texture view
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	dsvd.Format = depthTexDesc.Format;
	dsvd.Flags = 0;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Texture2D.MipSlice = 0;

	if (FAILED(device->CreateDepthStencilView(_depthTexPtr, &dsvd, &_depthStencilViews)))
	{
		OutputDebugStringA("Can't create cube map texture. \n");
		exit(524);
	}

	// Fill in the viewport data
	_viewport.Width = (float)_edgeLength;
	_viewport.Height = (float)_edgeLength;
	_viewport.MinDepth = 0.0f;
	_viewport.MaxDepth = 1.0f;
	_viewport.TopLeftX = 0;
	_viewport.TopLeftY = 0;

	_projMatrix = DirectX::XMMatrixPerspectiveFovLH(PI * 0.5f, 1.0f, 0.1f, 500.0f); //@TODO n and f should be params!
}



// Fills in view matrices for all six cameras, only needs to be called if the reflecting object moved
void CubeMapper::updateCams(const SVec3& pos)
{
	_cameras[0] = DirectX::XMMatrixLookAtLH(pos, SVec3(pos.x + 1.f, pos.y, pos.z), SVec3::Up);
	_cameras[1] = DirectX::XMMatrixLookAtLH(pos, SVec3(pos.x - 1.f, pos.y, pos.z), SVec3::Up);
	_cameras[2] = DirectX::XMMatrixLookAtLH(pos, SVec3(pos.x, pos.y + 1.f, pos.z), SVec3::Forward);		//flipped because simplemath...
	_cameras[3] = DirectX::XMMatrixLookAtLH(pos, SVec3(pos.x, pos.y - 1.f, pos.z), SVec3::Backward);	//is right handed by default
	_cameras[4] = DirectX::XMMatrixLookAtLH(pos, SVec3(pos.x, pos.y, pos.z + 1.f), SVec3::Up);
	_cameras[5] = DirectX::XMMatrixLookAtLH(pos, SVec3(pos.x, pos.y, pos.z - 1.f), SVec3::Up);
}



// Advances to the next render target view, clears it and the depth stencil view (it's reused for all of them)
void CubeMapper::advance(ID3D11DeviceContext* dc, UINT i)
{
	dc->RSSetViewports(1, &(_viewport));
	dc->ClearRenderTargetView(_renderTargetViews[i], _clearColour);
	dc->ClearDepthStencilView(_depthStencilViews, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	dc->OMSetRenderTargets(1, &_renderTargetViews[i], _depthStencilViews);
}



Camera CubeMapper::getCameraAtIndex(unsigned int i)
{
	return Camera::CreateFromViewProjection(_cameras[i], _projMatrix);
}