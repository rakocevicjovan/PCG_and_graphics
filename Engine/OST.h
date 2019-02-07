#pragma once

#include <d3d11.h>
#include "Math.h"
#include <vector>

class OST{

public:
	OST();
	~OST();

	ID3D11Texture2D* ostId;
	ID3D11ShaderResourceView* srv;
	ID3D11RenderTargetView* rtv;

	void Init(ID3D11Device* device, unsigned int w, unsigned int h, bool CPUAccessible = false);
	void SetRenderTarget(ID3D11DeviceContext*, ID3D11DepthStencilView*);
	void ClearRenderTarget(ID3D11DeviceContext*, ID3D11DepthStencilView*, float*);
	bool LoadToCpu(ID3D11Device* device, ID3D11DeviceContext*, std::vector<SVec4>& result);

	SMatrix _view, _lens;
	float _fov, _ar;
	bool isCPUAccessible;
	unsigned int _w, _h;
};

