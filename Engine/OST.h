#pragma once

#include <d3d11.h>

class OST{

public:
	OST();
	~OST();

	ID3D11Texture2D* ostId;
	ID3D11ShaderResourceView* srv;
	ID3D11RenderTargetView* rtv;

	void Init(ID3D11Device* device, int w, int h);
	void SetRenderTarget(ID3D11DeviceContext*, ID3D11DepthStencilView*);
	void ClearRenderTarget(ID3D11DeviceContext*, ID3D11DepthStencilView*, float*);

private:
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	D3D11_RENDER_TARGET_VIEW_DESC rtvd;
	D3D11_TEXTURE2D_DESC texDesc;
};

