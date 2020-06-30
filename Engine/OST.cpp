#include "OST.h"
#include "D3D.h"
#include "Texture.h"

OST::OST()
{
	_ostId = nullptr;
	_srv = nullptr;
	_rtv = nullptr;
}



OST::~OST()
{
	_ostId->Release();
	_srv->Release();
	_rtv->Release();
	_ostDepthId->Release();
	_ostDepthStencilView->Release();
}



void OST::init(ID3D11Device* device, unsigned int w, unsigned int h, DXGI_FORMAT format)
{
	_w = w;
	_h = h;

	// Create a 2d texture as a render target and a shader resource
	D3D11_TEXTURE2D_DESC texDesc = Texture::create2DTexDesc(w, h, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_USAGE_DEFAULT, 
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);

	if (FAILED(device->CreateTexture2D(&texDesc, 0, &_ostId)))
	{
		OutputDebugStringA("Can't create off-screen texture. \n");
		exit(420);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	ZeroMemory(&srvd, sizeof(srvd));
	srvd.Format = texDesc.Format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MostDetailedMip = 0u;
	srvd.Texture2D.MipLevels = 1u;

	if (FAILED(device->CreateShaderResourceView(_ostId, &srvd, &_srv)))
	{
		OutputDebugStringA("Can't create shader resource view. \n");
		exit(421);
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvd;
	ZeroMemory(&rtvd, sizeof(rtvd));
	rtvd.Format = texDesc.Format;
	rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvd.Texture2D.MipSlice = 0;

	if (FAILED(device->CreateRenderTargetView(_ostId, &rtvd, &_rtv)))
	{
		OutputDebugStringA("Can't create render target view. \n");
		exit(422);
	}


	// Create a depth buffer for the off screen texture
	D3D11_TEXTURE2D_DESC ostDepthTexDesc;
	ZeroMemory(&ostDepthTexDesc, sizeof(ostDepthTexDesc));
	ostDepthTexDesc.Width = _w;
	ostDepthTexDesc.Height = _h;
	ostDepthTexDesc.MipLevels = 1;
	ostDepthTexDesc.ArraySize = 1;
	ostDepthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	ostDepthTexDesc.CPUAccessFlags = 0;
	ostDepthTexDesc.SampleDesc = { 1, 0 };
	ostDepthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	ostDepthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	ostDepthTexDesc.MiscFlags = 0;

	if (FAILED(device->CreateTexture2D(&ostDepthTexDesc, 0, &_ostDepthId)))
	{
		OutputDebugStringA("Can't create cube map depth texture. \n");
		exit(523);
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	dsvd.Format = DXGI_FORMAT_D32_FLOAT;
	dsvd.Flags = 0;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Texture2D.MipSlice = 0;

	if (FAILED(device->CreateDepthStencilView(_ostDepthId, &dsvd, &_ostDepthStencilView)))
	{
		OutputDebugStringA("Can't create cube map texture. \n");
		exit(524);
	}


	// Fill out the viewport data for this texture
	ostViewport = Viewport::createViewport((float)_w, (float)_h);
}



void OST::setAsRenderTarget(ID3D11DeviceContext* deviceContext)
{
	deviceContext->RSSetViewports(1, &(ostViewport));
	deviceContext->OMSetRenderTargets(1, &_rtv, _ostDepthStencilView);
	clear(deviceContext);
}



void OST::clear(ID3D11DeviceContext* deviceContext)
{
	deviceContext->ClearRenderTargetView(_rtv, _clearColour);
	deviceContext->ClearDepthStencilView(_ostDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	//deviceContext->ClearDepthStencilView(_ostDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}



bool OST::loadToCpu(ID3D11Device* device, ID3D11DeviceContext* dc, std::vector<unsigned char>& result)
{
	ID3D11Texture2D* stagingId = nullptr;

	D3D11_TEXTURE2D_DESC texDesc; //Texture::create2DTexDesc(_w, _h, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_USAGE_STAGING, 0u, D3D11_CPU_ACCESS_READ, 0u);

	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = _w;
	texDesc.Height = _h;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_STAGING;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	texDesc.BindFlags = 0;
	texDesc.MiscFlags = 0;

	// Create a temporary staging texture, same format as the original OST
	if (FAILED(device->CreateTexture2D(&texDesc, 0, &stagingId)))
	{
		OutputDebugStringA("Can't create off-screen texture. \n");
		exit(425);
	}

	// Copy data from the OST to the staging texture
	dc->CopyResource(stagingId, _ostId);

	D3D11_MAPPED_SUBRESOURCE msr;
	dc->Map(stagingId, 0, D3D11_MAP_READ, 0, &msr);
	std::vector<float>tempFloatArr((float *)msr.pData, (float*)msr.pData + (msr.DepthPitch / 4));	//careful, depends on format!
	dc->Unmap(stagingId, 0);

	result.reserve(tempFloatArr.size());
	for (float& val : tempFloatArr)
	{
		unsigned char colChar = val * 255;
		result.push_back(colChar);
	}

	stagingId->Release();
}


// This is slow, intended for debugging
void OST::saveToFile(D3D& d3d, const std::string& filepath)
{
	std::vector<unsigned char> wat;
	loadToCpu(d3d.GetDevice(), d3d.GetDeviceContext(), wat);
	Texture::WriteToFile(filepath, _w, _h, 4, wat.data(), 0);	//4 here also depends on format...
}