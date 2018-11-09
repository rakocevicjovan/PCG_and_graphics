#pragma once

#include <d3d11.h>
#include "Math.h"
#include <vector>

struct Colour{
	unsigned char col[4];

	Colour(int r, int g, int b, int a) {
		col[0] = (unsigned char)r;
		col[1] = (unsigned char)g;
		col[2] = (unsigned char)b;
		col[3] = (unsigned char)a;
	}
};

class CubeMapper{

public:
	CubeMapper();
	~CubeMapper();

private:


	ID3D11Texture2D *cm_id, *cm_depth_id;
	ID3D11ShaderResourceView* cm_srv;
	ID3D11RenderTargetView* cm_rtv;
	ID3D11DepthStencilView* cm_depthStencilView;

	void BuildDynamicCubeMapViews(ID3D11Device* device, unsigned int width, unsigned int height) {

		HRESULT res;
		
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 6;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.CPUAccessFlags = 0;
		texDesc.SampleDesc = { 1, 0 };
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		res = device->CreateTexture2D(&texDesc, 0, &cm_id);
		if (FAILED(res)) {
			OutputDebugStringA("Can't create cube map texture. \n");
			exit(520);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
		srvd.Format = texDesc.Format;
		srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvd.TextureCube.MipLevels = texDesc.MipLevels;
		srvd.TextureCube.MostDetailedMip = 0;

		res = device->CreateShaderResourceView(cm_id, &srvd, &cm_srv);
		if (FAILED(res)) {
			OutputDebugStringA("Can't create shader resource view. \n");
			exit(521);
		}

		D3D11_RENDER_TARGET_VIEW_DESC rtvd;
		rtvd.Format = texDesc.Format;
		rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvd.Texture2D.MipSlice = 0;

		res = device->CreateRenderTargetView(cm_id, &rtvd, &cm_rtv);
		if (FAILED(res)) {
			OutputDebugStringA("Can't create render target view. \n");
			exit(522);
		}

		D3D11_TEXTURE2D_DESC depthTexDesc;
		ZeroMemory(&depthTexDesc, sizeof(depthTexDesc));
		depthTexDesc.Width = width;
		depthTexDesc.Height = height;
		depthTexDesc.MipLevels = 1;
		depthTexDesc.ArraySize = 6;
		depthTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		depthTexDesc.CPUAccessFlags = 0;
		depthTexDesc.SampleDesc = { 1, 0 };
		depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
		depthTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		depthTexDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	}


	
};

