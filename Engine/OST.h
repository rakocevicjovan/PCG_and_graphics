#pragma once

#include <d3d11.h>
#include "Math.h"

class OST{

public:
	OST();
	~OST();

	ID3D11Texture2D* ostId;
	ID3D11ShaderResourceView* srv;
	ID3D11RenderTargetView* rtv;

	void Init(ID3D11Device* device, unsigned int w, unsigned int h);
	void SetRenderTarget(ID3D11DeviceContext*, ID3D11DepthStencilView*);
	void ClearRenderTarget(ID3D11DeviceContext*, ID3D11DepthStencilView*, float*);

	SMatrix _view, _lens;
	float _fov, _ar;

private:
};

