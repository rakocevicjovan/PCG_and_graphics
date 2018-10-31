#include "OST.h"



OST::OST(){

	ostId = nullptr;
	srv = nullptr;
	rtv = nullptr;
}


OST::~OST(){
	ostId->Release();
	srv->Release();
	rtv->Release();
	//required?
	//delete ostId;
	//delete srv;
	//delete rtv;
}

void OST::Init(ID3D11Device* device, unsigned int w, unsigned int h) {

	HRESULT res;

	ZeroMemory(&texDesc, sizeof(texDesc));

	texDesc.Width = w;
	texDesc.Height = h;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	res = device->CreateTexture2D(&texDesc, 0, &ostId);
	if (FAILED(res)) {
		OutputDebugStringA("Can't create off-screen texture. \n");
		exit(420);
	}


	srvd.Format = texDesc.Format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MostDetailedMip = 0;
	srvd.Texture2D.MipLevels = 1;

	res = device->CreateShaderResourceView(ostId, &srvd, &srv);	//&resViewDesc
	if (FAILED(res)) {
		OutputDebugStringA("Can't create shader resource view. \n");
		exit(421);
	}


	rtvd.Format = texDesc.Format;
	rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvd.Texture2D.MipSlice = 0;

	res = device->CreateRenderTargetView(ostId, &rtvd, &rtv);	//&resViewDesc
	if (FAILED(res)) {
		OutputDebugStringA("Can't create render target view. \n");
		exit(422);
	}

}

void OST::SetRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView){
	deviceContext->OMSetRenderTargets(1, &rtv, depthStencilView);
}

void OST::ClearRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView, float* color){

	deviceContext->ClearRenderTargetView(rtv, color);
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}