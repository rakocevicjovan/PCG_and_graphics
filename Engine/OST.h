#pragma once

#include "Texture.h"
#include <d3d11.h>
#include "Math.h"
#include <vector>
#include <string>

class D3D;
class ShaderDepth;
class Camera;

class OST
{
private:
	float ccb[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

public:
	OST();
	~OST();

	ID3D11Texture2D* _ostId, *_ostDepthId;
	ID3D11ShaderResourceView* _srv;
	ID3D11RenderTargetView* _rtv;
	ID3D11DepthStencilView* _ostDepthStencilView;

	void init(ID3D11Device* dev, unsigned int w, unsigned int h);
	void SetRenderTarget(ID3D11DeviceContext*);	//, ID3D11DepthStencilView*
	void ClearRenderTarget(ID3D11DeviceContext*);
	bool LoadToCpu(ID3D11Device* dev, ID3D11DeviceContext*, std::vector<unsigned char>& result);
	void SaveToFile(D3D& d3d, const std::string& filepath);
	float* getClearColourPointer() { return ccb; }

	D3D11_VIEWPORT ostViewport;
	unsigned int _w, _h;
};