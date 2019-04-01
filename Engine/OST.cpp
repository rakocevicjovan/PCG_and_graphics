#include "OST.h"
#include "D3D.h"
#include "Model.h"
#include "ShaderDepth.h"
#include "Camera.h"


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

	if (FAILED(device->CreateTexture2D(&texDesc, 0, &ostId)))
	{
		OutputDebugStringA("Can't create off-screen texture. \n");
		exit(420);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	ZeroMemory(&srvd, sizeof(srvd));
	srvd.Format = texDesc.Format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MostDetailedMip = 0;
	srvd.Texture2D.MipLevels = 1;

	if (FAILED(device->CreateShaderResourceView(ostId, &srvd, &srv)))
	{
		OutputDebugStringA("Can't create shader resource view. \n");
		exit(421);
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvd;
	ZeroMemory(&rtvd, sizeof(rtvd));
	rtvd.Format = texDesc.Format;
	rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvd.Texture2D.MipSlice = 0;

	if (FAILED(device->CreateRenderTargetView(ostId, &rtvd, &rtv)))
	{
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



void OST::DrawDepthToTexture(D3D& d3d, std::vector<Model*>& models, ShaderDepth& sd, Camera& c)
{
	d3d.GetDeviceContext()->OMSetRenderTargets(1, &(rtv), d3d.GetDepthStencilView());	//switch to drawing on ost for the prepass	
	d3d.GetDeviceContext()->ClearRenderTargetView(rtv, ccb);	//then clear it, both the colours and the depth-stencil buffer
	d3d.GetDeviceContext()->ClearDepthStencilView(d3d.GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	for (auto tm : models)
	{
		sd.SetShaderParameters(d3d.GetDeviceContext(), *tm, c.GetViewMatrix(), c.GetProjectionMatrix());	// offScreenTexture._view, offScreenTexture._lens
		tm->Draw(d3d.GetDeviceContext(), sd);
	}

}



bool OST::LoadToCpu(ID3D11Device* device, ID3D11DeviceContext* dc, std::vector<unsigned char>& result)
{
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
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	texDesc.BindFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* stagingId = nullptr;

	HRESULT res = device->CreateTexture2D(&texDesc, 0, &stagingId);
	if (FAILED(res))
	{
		OutputDebugStringA("Can't create off-screen texture. \n");
		exit(425);
	}

	dc->CopyResource(stagingId, ostId);

	D3D11_MAPPED_SUBRESOURCE msr;

	dc->Map(stagingId, 0, D3D11_MAP_READ, 0, &msr);
	
	std::vector<float>tempFloatArr((float *)msr.pData, (float*)msr.pData + (msr.DepthPitch / 4));

	dc->Unmap(stagingId, 0);

	result.reserve(tempFloatArr.size());
	for (float& val : tempFloatArr)
	{
		unsigned char colChar = val * 255;
		result.push_back(colChar);
	}

	stagingId->Release();
}



void OST::SaveToFile(D3D& d3d, const std::string& filepath)
{
	std::vector<unsigned char> wat;
	LoadToCpu(d3d.GetDevice(), d3d.GetDeviceContext(), wat);
	Texture::WriteToFile(filepath, _w, _h, 4, wat.data(), 0);	//_w * 4
}
