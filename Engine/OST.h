#pragma once

#include <d3d11.h>
#include "Math.h"
#include <vector>
#include <string>

class D3D;
class Model;
class ShaderDepth;
class Camera;

class OST{

private:
	float ccb[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

public:
	OST();
	~OST();

	ID3D11Texture2D* ostId, *ostDepthId;
	ID3D11ShaderResourceView* srv;
	ID3D11RenderTargetView* rtv;
	ID3D11DepthStencilView* ostDepthStencilView;

	void Init(ID3D11Device* device, unsigned int w, unsigned int h, bool CPUAccessible = false);
	void SetRenderTarget(ID3D11DeviceContext*);	//, ID3D11DepthStencilView*
	void ClearRenderTarget(ID3D11DeviceContext*);
	bool LoadToCpu(ID3D11Device* device, ID3D11DeviceContext*, std::vector<unsigned char>& result);
	void DrawDepthToTexture(D3D& d3d, std::vector<Model*>& models, ShaderDepth& sd, Camera& c);
	void SaveToFile(D3D& d3d, const std::string& filepath);
	float* getClearColourPointer() { return ccb; }

	D3D11_VIEWPORT ostViewport;
	SMatrix _view, _lens;
	float _fov, _ar;
	bool isCPUAccessible;
	unsigned int _w, _h;
};

