#pragma once

#include <d3d11.h>
#include "Math.h"
#include <vector>
#include <string>

class D3D;
class Camera;

class OST
{
private:
	ID3D11Texture2D* _ostId;
	ID3D11ShaderResourceView* _srv;
	ID3D11RenderTargetView* _rtv;
	
	ID3D11Texture2D* _ostDepthId;
	ID3D11DepthStencilView* _ostDepthStencilView;

	D3D11_VIEWPORT ostViewport;
	unsigned int _w, _h;

	float _clearColour[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	float* getClearColourPointer() { return _clearColour; }

public:
	OST();
	~OST();

	void init(ID3D11Device* dev, unsigned int w, unsigned int h, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
	void setAsRenderTarget(ID3D11DeviceContext*);
	void clear(ID3D11DeviceContext*);
	bool loadToCpu(ID3D11Device* dev, ID3D11DeviceContext*, std::vector<unsigned char>& result);
	void saveToFile(D3D& d3d, const std::string& filepath);
};