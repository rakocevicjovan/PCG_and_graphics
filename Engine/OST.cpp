#include "OST.h"



OST::OST()
{
	ostId = nullptr;
	srv = nullptr;
	rtv = nullptr;
}



OST::~OST()
{
	ostId->Release();
	srv->Release();
	rtv->Release();
}



void OST::Init(ID3D11Device* device, unsigned int w, unsigned int h, bool CPUAccessible) 
{

	HRESULT res;
	isCPUAccessible = CPUAccessible;
	_w = w;
	_h = h;

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));

	texDesc.Width = w;
	texDesc.Height = h;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;

	if (CPUAccessible)
	{
		texDesc.Usage = D3D11_USAGE_DYNAMIC;
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	}
	else
	{
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.CPUAccessFlags = 0;
	}
		

	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.MiscFlags = 0;

	res = device->CreateTexture2D(&texDesc, 0, &ostId);
	if (FAILED(res)) {
		OutputDebugStringA("Can't create off-screen texture. \n");
		exit(420);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	ZeroMemory(&srvd, sizeof(srvd));
	srvd.Format = texDesc.Format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MostDetailedMip = 0;
	srvd.Texture2D.MipLevels = 1;

	res = device->CreateShaderResourceView(ostId, &srvd, &srv);	//&resViewDesc
	if (FAILED(res)) {
		OutputDebugStringA("Can't create shader resource view. \n");
		exit(421);
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvd;
	ZeroMemory(&rtvd, sizeof(rtvd));
	rtvd.Format = texDesc.Format;
	rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvd.Texture2D.MipSlice = 0;

	res = device->CreateRenderTargetView(ostId, &rtvd, &rtv);	//&resViewDesc
	if (FAILED(res)) {
		OutputDebugStringA("Can't create render target view. \n");
		exit(422);
	}

	_ar = float(w) / (float)h;
	_fov = PI * 0.5f;
}



void OST::SetRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView)
{
	deviceContext->OMSetRenderTargets(1, &rtv, depthStencilView);
}



void OST::ClearRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView, float* color)
{

	deviceContext->ClearRenderTargetView(rtv, color);
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}



bool OST::LoadToCpu(ID3D11Device* device, ID3D11DeviceContext* dc, std::vector<SVec4>& result)
{
	//if (!isCPUAccessible) return false;

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));

	texDesc.Width = _w;
	texDesc.Height = _h;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_STAGING;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	texDesc.BindFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* stagingId = nullptr;

	HRESULT res = device->CreateTexture2D(&texDesc, 0, &stagingId);
	if (FAILED(res)) {
		OutputDebugStringA("Can't create off-screen texture. \n");
		exit(425);
	}

	D3D11_BOX boxbox;
	boxbox.left = 0;
	boxbox.right = _w;
	boxbox.top = 0;
	boxbox.bottom = _h;
	boxbox.front = 0;
	boxbox.back = 0;

	dc->CopySubresourceRegion(stagingId, 0, 0, 0, 0, ostId, 0, &boxbox);

	result.reserve(_w * _h);

	D3D11_MAPPED_SUBRESOURCE msr;

	dc->Map(stagingId, 0, D3D11_MAP_READ, 0, &msr);

	memcpy(result.data(), msr.pData, result.size());

	dc->Unmap(stagingId, 0);
	stagingId->Release();
}
