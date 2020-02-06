#ifndef _D3D_H_
#define _D3D_H_

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include "Math.h"

class D3D
{

public:
	D3D();
	~D3D();

	bool Initialize(int, int, bool, HWND, bool, float, float);
	void Shutdown();
	
	void ClearColourDepthBuffers();
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetVideoCardInfo(char*, int&);
	ID3D11DepthStencilView* GetDepthStencilView();
	void SetBackBufferRenderTarget();

	void TurnOnAlphaBlending();
	void TurnOffAlphaBlending();

	void setRSSolidCull();
	void setRSSolidNoCull();
	void setRSWireframe();

	void SwitchDepthToLessEquals();
	void SwitchDepthToDefault();

	ID3D11RenderTargetView* _renderTargetView;
	D3D11_VIEWPORT _viewport;
	float _clearColour[4] = { 0.2f, 0.1f, 0.3f, 1.0f };

private:
	bool _vsync_enabled;
	int _videoCardMemory;
	char _videoCardDescription[128];
	IDXGISwapChain* _swapChain;
	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;
	ID3D11Texture2D* _depthStencilBuffer;
	ID3D11DepthStencilState* _depthStencilLess, *_depthStencilLessEquals;
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11RasterizerState* _r_s_solid_cull, *_r_s_solid_no_cull, *_r_s_wireframe;
	ID3D11BlendState* _blendState, *_noBlendState;


public:

	inline static D3D11_VIEWPORT createViewport(float w, float h, float minD = 0.f, float maxD = 1.f, float tlX = 0.f, float tlY = 0.f)
	{
		D3D11_VIEWPORT viewport;

		viewport.Width = w;
		viewport.Height = h;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;

		return viewport;
	}
};

#endif